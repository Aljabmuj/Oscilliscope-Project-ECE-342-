#include <ADC.h>
#include <AnalogBufferDMA.h>

// Define adc and buffer size
ADC * adc = new ADC();
const uint32_t buffer_size = 20000;

// Define pins 
int channel0 = 14;
int channel1 = 15;

// Define varibles/constants for calculations
int max0 = 0;
int max1 = 0;
float Vpp0 = 0;
float Vpp1 = 0;
int buffer0[501];
int buffer1[501];
int channel0_on = true;
int channel1_on = false;
int run = true;

// Define trigger values for channel 1 and 2
int arming_level0 = 600;
int triggering_level0 = arming_level0 + 5;
int arming_level1 = 600;
int triggering_level1 = arming_level1 + 5;





// Allocate memory
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
  // Set pinModes, baud rate, adc, and buffer
  pinMode(channel0, INPUT_DISABLE);
  pinMode(channel1, INPUT_DISABLE);
  pinMode(run_stop, INPUT);
  attachInterrupt(digitalPinToInterrupt(run_stop), Run_Stop, FALLING);

  // Define baud rate
  Serial.begin(4608000);

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
  adc->adc0->startContinuous(channel1);

  // Establish connection
  establishConnection();

  
  delay(10);

}

void loop() {
  // Pooling to see if buffer is full
  if ((abdma1.interrupted()) && (abdma2.interrupted())){
       if ((abdma1.interrupted())){
          Process_Buffer(&abdma1, 0);
       }
       if ((abdma2.interrupted())){
          Process_Buffer(&abdma2, 1);
       }
  }
}


void Process_Buffer(AnalogBufferDMA *pabdma, int adc_num){
  // Find the start and end address in memory that the buffer stored samples values
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  volatile uint16_t *end_buffer = pbuffer + pabdma->bufferCountLastISRFilled();

  // Check if at the end of direct access memory
  if ((uint32_t)pbuffer >= 0x20200000u){
    arm_dcache_delete((void *)pbuffer, sizeof(dma_adc_buff1));
  }
  while (pbuffer < end_buffer){

      // See if value is in trigger
      int value = *pbuffer;
      if (adc_num == 0){
        max0 = max(max0, value);
      }
      else{
        max1 = max(max1, value);
      }
      
      
      if ((value >= arming_level) && (value >= triggering_level)){
        // If value in trigger fill buffer
        for(int i = 0; i<500; i++){
          buffer0[i] = *pbuffer;
          max0 = max(max0, *pbuffer);
          pbuffer=pbuffer+1;
        }
        break;
      }

    pbuffer ++;
  }

  // Export buffer
  ExportBuffer();

  // Caclulate peak to peak voltgae
  Vpp0 = ((max0 * 3.3 * 2)/1023.0) - (2 * 1.65) + .28;
  
  // Reset max and see if Processing needes new data
  max0= 0;
  while ((Serial.available() == 0) || (run !=true)){
  }
  // Read and clear interrupt
  int inByte = Serial.read();
  pabdma->clearInterrupt();
}


void ExportBuffer(){
    // If cahnnel 1 is on export data to Processing
    if(channel0_on){
      Serial.print('R');
      Serial.print(',');
      for (int x = 0; x<500; x++){
      Serial.print(buffer0[x]);
      Serial.print(',');
    }
      // Export the peak to peak voltage to Processing
      Serial.print(Vpp0);
      Serial.println();
    }

    // If channel 2 is on export data to Processing
    if (channel1_on){
      Serial.print('L');
      Serial.print(',');
      for (int x = 0; x<500; x++){
        Serial.print(buffer1[x]);
        Serial.print(',');
      }
      // Export the peak to peak voltage to Processing
      Serial.print(Vpp1);
      Serial.println();
    }
   
}


void establishConnection(){
  // Establich connection
  while(Serial.available() <= 0){
    Serial.write('A');
    delay(30);
  }
}

void Run_Stop(){
  // See if stop or start is wanted
  run = !run;
  delay(15);
}




