#include "BNumberWidget.h"
#include "Display.h"
#include "Controls.h"
#include "cmath"
#include <strings.h>

BNumberWidget::BNumberWidget(char *aTitle, const TRange *aRange, const TPad *aPad, TInt aForeground, TInt aBackground)
    : BWidget(aTitle), mRange(aRange), mPad(aPad),
      mIsFloat(aRange->precision > 0),
      mIsPadded(aPad && aPad->padCount) {
  mSelectedValue = mRange->start;
}

BNumberWidget::BNumberWidget(const char *aTitle, const TRange *aRange, const TPad *aPad, TInt aForeground, TInt aBackground)
    : BWidget(), mRange(aRange), mPad(aPad),
      mIsFloat(aRange->precision > 0),
      mIsPadded(aPad && aPad->padCount) {
  mSelectedValue = mRange->start;
  mTitle = (char *) aTitle;
  mHeight = 40;
}

BNumberWidget::~BNumberWidget() {}

TInt BNumberWidget::Render(TInt aX, TInt aY) {
  const BFont *font = gWidgetTheme.GetFont(WIDGET_TEXT_FONT);
  const TInt  fg    = gWidgetTheme.GetInt(WIDGET_TEXT_FG),
              bg    = gWidgetTheme.GetInt(WIDGET_TEXT_BG);
  char        c[20];

  // Count number of digits in whole part
  TUint digitCount = 0;
  TUint decimalCount = 0;
  TUint32 n = TUint32(mSelectedValue);
  do {
    digitCount++;
    n /= 10;
  } while (n);

  if (mIsFloat) {
    // Count number of decimals (incl separator)
    n = mRange->precision;
    do {
      decimalCount++;
      n /= 10;
    } while (n);

    // Adjust padding for decimals and separator
    if (mPad->padCount > decimalCount) {
      digitCount = MAX(mPad->padCount - decimalCount, digitCount);
    }
  } else if(mIsPadded) {
    // Set padding
    digitCount = MAX(mPad->padCount, digitCount);
  }

  // Fill char array with pad chars
  const char fillChar = mIsPadded ? mPad->padChar : '0';
  TUint i = 0;
  for (; i < digitCount; i++) {
    c[i] = fillChar;
  }
  // Required for strlen to function across platforms
  c[i] = '\0';

  // Store current string length
  // Store a copy of strlen for writing the whole part right to left
  TUint valueStrLen = strlen(c);
  TUint valueStrLenTmp = valueStrLen;
  n = TUint32(mSelectedValue);
  c[--valueStrLenTmp] = '0' + n % 10;
  while(n >= 10) {
    n /= 10;
    c[--valueStrLenTmp] = '0' + n % 10;
  }

  if (mIsFloat) {
    // There are more than 1 decimal places
    if (mRange->precision > 10) {
      n = mRange->precision;
      c[valueStrLen] = '.';

      // Move the starting point for writing decimals from right to left
      while (n) {
        valueStrLen++;
        n /= 10;
      }

      // Write decimals right to left
      // starting from \0, until we hit the . separator
      TUint d = TUint32(round(mSelectedValue * mRange->precision)) % mRange->precision;
      c[valueStrLen] = '\0';
      c[--valueStrLen] = '0' + d % 10;
      while(c[valueStrLen - 1] != '.') {
        d /= 10;
        c[--valueStrLen] = '0' + d % 10;
      }
    } else {
      // Write separator, decimal and \0
      c[valueStrLen++] = '.';
      c[valueStrLen++] = '0' + TUint32(round(mSelectedValue * mRange->precision)) % mRange->precision;
      c[valueStrLen] = '\0';
    }
  }

  // Draw string
  gDisplay.renderBitmap->DrawString(ENull, c, font, aX * 2, aY, bg, fg);

  return font->mHeight;
}

void BNumberWidget::Run() {
  if (!IsActive()) {
    return;
  }

  // Decrement, check min value and convert to TFloat if needed
  if (gControls.WasPressed(JOYLEFT)) {
    if (mIsFloat) {
      mSelectedValue *= mRange->precision;
      mSelectedValue = MAX(mRange->start, mSelectedValue - mRange->step);
      mSelectedValue /= mRange->precision;
    } else {
      mSelectedValue = MAX(mRange->start, mSelectedValue - mRange->step);
    }
    return;
  }

  // Increment, check max value and convert to TFloat if needed
  if (gControls.WasPressed(JOYRIGHT)) {
    if (mIsFloat) {
      mSelectedValue *= mRange->precision;
      mSelectedValue = MIN(mRange->end, mSelectedValue + mRange->step);
      mSelectedValue /= mRange->precision;
    } else {
      mSelectedValue = MIN(mRange->end, mSelectedValue + mRange->step);
    }
    return;
  }

  // Store selected value
  if (gControls.WasPressed(BUTTON_SELECT)) {
    Select(mSelectedValue);
  }
}
