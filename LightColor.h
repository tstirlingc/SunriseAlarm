#ifndef LIGHTCOLOR_H
#define LIGHTCOLOR_H

namespace SunriseAlarm {

uint8_t computeMinRGBLevel(const uint8_t* RGB)
{
  uint8_t myRGB[3] = {255, 255, 255};
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    if (RGB[i] > 0) myRGB[i] = RGB[i];
  }
  return min(min(myRGB[0], myRGB[1]), myRGB[2]);
}


const uint8_t candleLight[3] = {255, 147, 41};
const uint8_t sunLight[3] = {255, 255, 255};
const uint8_t tungsten100W[3] = {255, 214, 170}; 
const uint8_t halogen[3] = {255, 241, 224};
const uint8_t testingLight[3] = {50, 0, 0};
uint8_t targetColor[3] = {255, 0, 0};
const uint8_t sunRiseDimLevel = 90; // Percentage of max [0, 100]

uint8_t minRGBLevel = computeMinRGBLevel(targetColor);
uint16_t totalDimmerSteps = minRGBLevel * NUM_LED;

void setColorForSunset()
{
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    targetColor[i] = (sunsetDimLevel*1.0/100.0)*(1.0*candleLight[i]);
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel * NUM_LED;
}

void setColorForSunRise()
{
  for (uint8_t i = 0 ; i < 3 ; ++i)
  {
    targetColor[i] = (sunRiseDimLevel*1.0/100.0)*(1.0*tungsten100W[i]);
  }
  minRGBLevel = computeMinRGBLevel(targetColor);
  totalDimmerSteps = minRGBLevel * NUM_LED;
}

} // namespace SunriseAlarm 

#endif // LIGHTCOLOR_H

