#include "ColorUtil.h"

namespace BF
{

/* internal functions to convert HSV to RGB */
float gHSVToRGB_rgbvalue (float n1, float n2, float hue)
{
	while (hue > 360.0f)
	hue -= 360.0f;
	while (hue < 0.0f)
	hue += 360.0f;
	
	if (hue < 60.0f)
	return n1 + (n2 - n1) * hue / 60.0f;
	else if (hue<180.0f)
	return n2;
	else if (hue < 240.0f)
	return n1 + (n2 - n1) * (240.0f - hue) / 60.0f;
	else
	return n1;
}


void gHSVToRGB(float inH, float inS, float inV, float& outR, float& outG, float& outB)
{
  if (inS < 1.0e-8) {
    outR = outG = outB = inV;
  }
  else {
    int i;
    float f, m, n, k;
    inH = fmod(inH, 1.0f) * 6.0f;
    i = (int) floor(inH);
    f = inH - (float) i;
    m = inV * (1 - inS);
    n = inV * (1 - inS * f);
    k = inV * (1 - inS * (1 - f));
    switch (i) {
    case 0:
      outR = inV; outG = k; outB = m;
      break;
    case 1:
      outR = n; outG = inV; outB = m;
      break;
    case 2:
      outR = m; outG = inV; outB = k;
      break;
    case 3:
      outR = m; outG = n; outB = inV;
      break;
    case 4:
      outR = k; outG = m; outB = inV;
      break;
    case 5:
      outR = inV; outG = m; outB = n;
      break;
    }
  }
}


GoldenRatioRandomColorGenerator::GoldenRatioRandomColorGenerator(RandomSeed inSeed)
{
	mValue = Random(inSeed).randf();
}

glm::vec3 GoldenRatioRandomColorGenerator::Next()
{
	//http://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
	static float golden_ratio_conjugate = 0.618033988749895f;

	mValue += golden_ratio_conjugate;
	while (mValue > 1.0f)
		mValue -= 1.0f;

	glm::vec3 ret;
	gHSVToRGB(mValue, 0.5f, 0.95f, ret.r, ret.g, ret.b);

	return ret;
}

}
