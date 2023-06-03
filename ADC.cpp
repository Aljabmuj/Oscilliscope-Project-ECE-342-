#include <ADC.h>
#include <AnalogBufferDMA.h>

// Define adc and buffer size
ADC * adc = new ADC();
const uint32_t buffer_size = 20000;

// Define pins 
#define DT_trigger 0
#define CLK_trigger 1
#define SW_trigger 2
#define DT_voltage 3
#define CLK_voltage 4
#define SW_voltage 5
#define DT_freq 6
#define CLK_freq 7
#define SW_freq 8
#define channel0 14
#define channel_0SW 17
#define channel1 15
#define channel_1SW 16

// Define varibles/constants for calculations
int max0 = 0;
int max1 = 0;
int min0 = 1024;
int min1 = 1024;
float Vpp0 = 0;
float Vpp1 = 0;

// Define the buffer for storing waveform values
int buffer0[500];
int buffer1[500];
int buffer_dual[1000];

// Varibles fow which channel is on and which channel trigger, voltage, and freq are on
int trigger_channel = true;
int voltage_channel = true;
int freq_channel = true;
int channel0_on = false;
int channel1_on = false;
int last_trigger;
int last_voltage;
int last_freq;
int current_trigger;
int current_voltage;
int current_freq;
int run = true;
int value;
int channel0_offset = 0;
int channel1_offset = 0;


// Define trigger values for channel 1 and 2
int arming_level0 = 507;
int triggering_level0 = arming_level0 + 2;
int arming_level1 = 507;
int triggering_level1 = arming_level1 + 2;

// Create values fro voltgae scaling, and freq scaling
int voltage_scalars[] = {1, 2, 3, 4, 5, 6, 7, 8};
int voltage_0 = 0;
int voltage_1 = 0;
int freq_0 = 1;
int freq_1 = 1;
float freq_0_temp;


// Allocate memory for both of the adc to write too when sampling
DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc_buff1[buffer_size];
DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc_buff2[buffer_size];
AnalogBufferDMA abdma1(dma_adc_buff1, buffer_size, dma_adc_buff2, buffer_size);


DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc2_buff1[buffer_size];
DMAMEM static volatile uint16_t __attribute__((aligned(32)))
dma_adc2_buff2[buffer_size];
AnalogBufferDMA abdma2(dma_adc2_buff1, buffer_size, dma_adc2_buff2, buffer_size);


// Start set up
void setup() {
  // Define baud rate
  Serial.begin(9600);
  // Set pinModes, baud rate, adc, and buffer
  //pinMode(channel0, INPUT_DISABLE);
  //pinMode(channel1, INPUT_DISABLE);


  pinMode(SW_trigger, INPUT);
  attachInterrupt(digitalPinToInterrupt(SW_trigger), trigger_cc, FALLING);
  attachInterrupt(DT_trigger, trigger_change, CHANGE);
  attachInterrupt(CLK_trigger, trigger_change, CHANGE);

  attachInterrupt(digitalPinToInterrupt(channel_1SW), channel1_cc, FALLING);
  attachInterrupt(digitalPinToInterrupt(channel_0SW), channel0_cc, FALLING);

  pinMode(SW_voltage, INPUT);
  attachInterrupt(digitalPinToInterrupt(SW_voltage), voltage_cc, FALLING);
  attachInterrupt(DT_voltage, voltage_change, CHANGE);
  attachInterrupt(CLK_voltage, voltage_change, CHANGE);


  pinMode(SW_freq, INPUT);
  attachInterrupt(digitalPinToInterrupt(SW_freq), freq_cc, FALLING);
  attachInterrupt(DT_freq, freq_change, CHANGE);
  attachInterrupt(CLK_freq, freq_change, CHANGE);


  // Set adc
  adc->adc0->setResolution(10);
  adc->adc0->setSamplingSpeed(
      ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc0->setConversionSpeed(
      ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc->adc1->setResolution(10);
  adc->adc1->setSamplingSpeed(
      ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc1->setConversionSpeed(
      ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
      

  // Set buffer
  abdma1.init(adc, ADC_0);
  abdma2.init(adc, ADC_1);

  // Start continuous
  adc->adc0->startContinuous(channel0);
  adc->adc1->startContinuous(channel1);


  // Establish connection
  establishConnection();

  delay(10);
}

void loop() {
  // Pooling to see if buffer is full
  if ((abdma1.interrupted()) && abdma2.interrupted()){
    if (channel0_on && channel1_on){
      Dual_Process(&abdma1, &abdma2);
    }
    else{
       if ((abdma1.interrupted()) && channel0_on){
          Process_Buffer0(&abdma1);
       }
       if (!channel0_on) Clear_Screen0();

       if ((abdma2.interrupted() && channel1_on)){
          Process_Buffer1(&abdma2);
       }
       if (!channel1_on) Clear_Screen1();
    }
  }
}

void Dual_Process(AnalogBufferDMA *pabdma1, AnalogBufferDMA *pabdma2){
  // Find start and end address in memeory for both buffers
  volatile uint16_t *pbuffer1 = pabdma1->bufferLastISRFilled();
  volatile uint16_t *end_buffer1 = pbuffer1 + pabdma1->bufferCountLastISRFilled();

  volatile uint16_t *pbuffer2 = pabdma2->bufferLastISRFilled();
  volatile uint16_t *end_buffer2 = pbuffer2 + pabdma2->bufferCountLastISRFilled();
  
  // Check if at the end of direct access memeory
  if ((uint32_t)pbuffer1 >= 0x20200000u){
    arm_dcache_delete((void *)pbuffer1, sizeof(dma_adc_buff1));
  }
  if ((uint32_t)pbuffer2 >= 0x20200000u){
    arm_dcache_delete((void *)pbuffer2, sizeof(dma_adc_buff1));
  }
  pbuffer1 = pbuffer1 + (250 * freq_0);
  while (pbuffer1 < end_buffer1){

      // See if value is in trigger
      int value = *pbuffer1;
      max0 = max(max0, value);
      min0 = min(min0, value);
      
      if ((value >= arming_level0) && (value >= triggering_level0)){
        // If value in trigger fill buffer
        pbuffer1 = pbuffer1 - (250 * freq_0);
        for(int i = 0; i<500; i++){
          value = *pbuffer1 - 512;
          value = value * voltage_scalars[voltage_0] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer_dual[i] = value;
          
          max0 = max(max0, *pbuffer1);
          min0 = min(min0, *pbuffer1);
          pbuffer1=pbuffer1+freq_0;
        }
        break;
      }
      if (pbuffer1 == (end_buffer1 - (250*freq_0))){
        pbuffer1 = pbuffer1 - (250 * freq_0);
        for(int i = 0; i<500; i++){
          value = *pbuffer1 - 512;
          value = value * voltage_scalars[voltage_0] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer_dual[i] = value;
          
          max0 = max(max0, *pbuffer1);
          min0 = min(min0, *pbuffer1);
          pbuffer1=pbuffer1+freq_0;
        }
        break;
      }
    pbuffer1 ++;
  }

  pbuffer2 = pbuffer2 + (250 * freq_1);
  while (pbuffer2 < end_buffer2){

      // See if value is in trigger
      int value = *pbuffer2;
      max1 = max(max1, value);
      min1 = min(min1, value);
      
      if ((value >= arming_level1) && (value >= triggering_level1)){
        // If value in trigger fill buffer
        pbuffer2 = pbuffer2 + (250 * freq_1);
        for(int i = 500; i<1000; i++){
          value = *pbuffer2 - 512;
          value = value * voltage_scalars[voltage_1] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer_dual[i] = value;

          max1 = max(max1, *pbuffer2);
          min1 = min(min1, *pbuffer2);
          pbuffer2=pbuffer2+freq_1;
        }
        break;
      }
      if (pbuffer2 == (end_buffer2 - (250*freq_1))){
        pbuffer2 = pbuffer2 - (250 * freq_1);
        for(int i = 500; i<1000; i++){
          value = *pbuffer2 - 512;
          value = value * voltage_scalars[voltage_1] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer_dual[i] = value;
          
          max1 = max(max1, *pbuffer2);
          min1 = min(min1, *pbuffer2);
          pbuffer2=pbuffer2+freq_1;
        }
        break;
      }

    pbuffer2 ++;
  }

  // Caclulate peak to peak voltage
  channel0_offset = 512 - (max0 + min0)/2;
  Vpp0 = (((max0 + channel0_offset) * 3.3 * 2)/1023.0) - (2 * 1.65);

  // Caclulate peak to peak voltage
  channel1_offset = 512 - (max1 + min1)/2;
  Vpp1 = (((max1 + channel1_offset) * 3.3 * 2)/1023.0) - (2 * 1.65);

  Dual_Export();
  max0 = 0;
  max1 = 0;
  min0 = 1024;
  min1 = 1024;
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
  pabdma1->clearInterrupt();
  pabdma2->clearInterrupt();
}

void Dual_Export(){
  // If cahnnel 1 is on export data to Processing
      Serial.print('C');
      Serial.print(',');

      for (int x = 0; x<500; x++){
      Serial.print(1024 - buffer_dual[x] - channel0_offset);
      Serial.print(',');
      }
      for (int x = 500; x<1000; x++){
      Serial.print(1024 - buffer_dual[x] - channel1_offset);
      Serial.print(',');
      }
      // Export the peak to peak voltage to Processing
      Serial.print(Vpp0);
      Serial.print(',');
      Serial.print(Vpp1);
      Serial.print(',');
      if (trigger_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (voltage_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (freq_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.println(); 
}

void Process_Buffer0(AnalogBufferDMA *pabdma){
  // Find the start and end address in memory that the buffer stored samples values
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  volatile uint16_t *end_buffer = pbuffer + pabdma->bufferCountLastISRFilled();

  // Check if at the end of direct access memory
  if ((uint32_t)pbuffer >= 0x20200000u){
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  }
  pbuffer = pbuffer + (250 * freq_0);
  while (pbuffer < end_buffer){

      // See if value is in trigger
      int value = *pbuffer;
      max0 = max(max0, value);
      min0 = min(min0, value);
      
      if ((value >= arming_level0) && (value >= triggering_level0)){
        // If value in trigger fill buffer
        pbuffer = pbuffer - (250 * freq_0);
        for(int i = 0; i<500; i++){
          value = *pbuffer - 512;
          value = value * voltage_scalars[voltage_0] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer0[i] = value;
          max0 = max(max0, *pbuffer);
          min0 = min(min0, *pbuffer);
          pbuffer=pbuffer+freq_0;
        }
        break;
      }
      if (pbuffer == (end_buffer - (250*freq_0))){
        pbuffer = pbuffer - (250 * freq_0);
        for(int i = 0; i<500; i++){
          value = *pbuffer - 512;
          value = value * voltage_scalars[voltage_0] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer0[i] = value;
          max0 = max(max0, *pbuffer);
          min0 = min(min0, *pbuffer);
          pbuffer=pbuffer+freq_0;
        }
        break;
      }
    pbuffer ++;
  }

  // Caclulate peak to peak voltgae
  channel0_offset = 512 - (max0 + min0)/2;
  Vpp0 = (((max0+channel0_offset) * 3.3 * 2)/1023.0) - (2*1.65);
  

  // Export buffer
  ExportBuffer0();
  
  // Reset max and see if Processing needes new data
  max0= 0;
  min0 = 1024;
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
  pabdma->clearInterrupt();
}

void ExportBuffer0(){
    // If cahnnel 1 is on export data to Processing
      Serial.print('R');
      Serial.print(',');
      for (int x = 0; x<500; x++){
        Serial.print(1024 - buffer0[x] - channel0_offset);
        Serial.print(',');
      }
      // Export the peak to peak voltage to Processing
      Serial.print(Vpp0);
      Serial.print(',');
      if (trigger_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (voltage_channel){
        Serial.print("Channel 1");      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (freq_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.println(); 
}

void Process_Buffer1(AnalogBufferDMA *pabdma){
  // Find the start and end address in memory that the buffer stored samples values
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  volatile uint16_t *end_buffer = pbuffer + pabdma->bufferCountLastISRFilled();

  // Check if at the end of direct access memory
  if ((uint32_t)pbuffer >= 0x20200000u){
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  }
  pbuffer = pbuffer + (250 * freq_1);
  while (pbuffer < end_buffer){

      // See if value is in trigger
      int value = *pbuffer;
      max1 = max(max1, value);
      min1 = min(min1, value);
      
      if ((value >= arming_level1) && (value >= triggering_level1)){
        // If value in trigger fill buffer
        pbuffer = pbuffer - (250 * freq_1);
        for(int i = 0; i<500; i++){
          value = *pbuffer - 512;
          value = value * voltage_scalars[voltage_1] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer1[i] = value;
          max1 = max(max1, *pbuffer);
          min1 = min(min1, *pbuffer);
          pbuffer=pbuffer+freq_1;
        }
        break;
      }
      if (pbuffer == (end_buffer - (250*freq_1))){
        pbuffer = pbuffer - (250 * freq_1);
        for(int i = 0; i<500; i++){
          value = *pbuffer - 512;
          value = value * voltage_scalars[voltage_1] + 512;
          if (value > 1024) value = 1024;
          if (value < 0) value = 0;
          buffer1[i] = value;
          max1 = max(max1, *pbuffer);
          min1 = min(min1, *pbuffer);
          pbuffer=pbuffer+freq_1;
        }
        break;
      }
    pbuffer ++;
  }

  // Caclulate peak to peak voltgae
  channel1_offset = 512 - (max1 + min1)/2;
  Vpp1 = (((max1+channel1_offset) * 3.3 * 2)/1023.0) - (2*1.65);
  

  // Export buffer
  ExportBuffer1();
  
  // Reset max and see if Processing needes new data
  max1= 0;
  min1 = 1024;
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
  pabdma->clearInterrupt();
}

void ExportBuffer1(){
    // If cahnnel 1 is on export data to Processing
      Serial.print('L');
      Serial.print(',');
      for (int x = 0; x<500; x++){
      Serial.print(1024 - buffer1[x] - channel1_offset);
      Serial.print(',');
      }
      // Export the peak to peak voltage to Processing
      Serial.print(Vpp1);
      Serial.print(',');
      if (trigger_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (voltage_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.print(',');
      if (freq_channel){
        Serial.print("Channel 1");
      }
      else{
        Serial.print("Channel 2");
      }
      Serial.println(); 
}

void establishConnection(){
  // Establich connection
  while(Serial.available() <= 0){
    Serial.write('A');
    delay(30);
  }
}

void trigger_change(){
  current_trigger = digitalRead(CLK_trigger);
  if (current_trigger != last_trigger && current_trigger == 1){
    if (digitalRead(DT_trigger) != current_trigger){
      if (trigger_channel){
        if (arming_level0 < 1023) arming_level0 = arming_level0 + 1;
      }
      else{
        if (arming_level1 < 1023) arming_level1 = arming_level1 + 1;
      }
    }
    else{
      if (trigger_channel){
        if (arming_level0 > 1) arming_level0 = arming_level0 - 1;
      }
      else{
        if (arming_level1 > 1) arming_level1 = arming_level1 - 1;
      }
    }
  }
  triggering_level0 = arming_level0 + 2;
  triggering_level1 = arming_level1 + 2;
  last_trigger = current_trigger;
}

void voltage_change(){
  current_voltage = digitalRead(CLK_voltage);
  if (current_voltage != last_voltage && current_voltage == 1){
    if (digitalRead(DT_voltage) != current_voltage){
      if (voltage_channel){
        if (voltage_0 < 7) voltage_0++;
      }
      else{
        if (voltage_1 < 7) voltage_1++;
      }
    }
    else{
      if (voltage_channel){
        if (voltage_0 > 0) voltage_0--;
      }
      else{
        if (voltage_1 > 0) voltage_1--;
      }
    }
  }
  last_voltage = current_voltage;
}

void freq_change(){
  current_freq = digitalRead(CLK_freq);
  if (current_freq != last_freq && current_freq == 1){
    if (digitalRead(DT_freq) != current_freq){
      if (freq_channel){
        if (freq_0 < 40) freq_0++;
      }
      else{
        if (freq_1 < 40) freq_1++;
      }
    }
    else{
      if (freq_channel){
        if (freq_0 > 1) freq_0--;
      }
      else{
        if (freq_1 > 1) freq_1--;
      }
    }
  }
  last_freq = current_freq;
}

void trigger_cc(){
  trigger_channel =! trigger_channel;
  delay(15);
}

void voltage_cc(){
 voltage_channel =! voltage_channel;
  delay(15);
}

void freq_cc(){
  freq_channel =! freq_channel;
  delay(15);
}

void channel1_cc(){
  channel1_on =! channel1_on;
  delay(15);
}

void channel0_cc(){
  channel0_on =! channel0_on;
  delay(15);
}

void Clear_Screen0(){
  Serial.print('J');
  Serial.print(',');
  for (int x = 0; x<500; x++){
    Serial.print(0);
    Serial.print(',');
  }
  // Export the peak to peak voltage to Processing
  Serial.print('?');
  Serial.println();
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
}

void Clear_Screen1(){
  Serial.print('K');
  Serial.print(',');
  for (int x = 0; x<500; x++){
    Serial.print(0);
    Serial.print(',');
  }
  // Export the peak to peak voltage to Processing
  Serial.print('?');
  Serial.println();
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
}



