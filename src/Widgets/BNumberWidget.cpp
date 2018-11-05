#include "BNumberWidget.h"
#include "Display.h"
#include "Controls.h"
#include <strings.h>

BNumberWidget::BNumberWidget(char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground)
    : BWidget(aTitle), mRange(aRange), mIsFloat(aRange->precision > 0) {
  mSelectedValue = mRange->start;
}

BNumberWidget::BNumberWidget(const char *aTitle, const TRange *aRange, TInt aForeground, TInt aBackground)
    : BWidget(), mRange(aRange), mIsFloat(aRange->precision > 0) {
  mSelectedValue = mRange->start;
  mTitle = (char *) aTitle;
  mHeight = 40;
}

BNumberWidget::~BNumberWidget() {}

TInt BNumberWidget::Render(TInt aX, TInt aY) {
  const BFont *font = gWidgetTheme.GetFont(WIDGET_TEXT_FONT);
  const TInt  fg    = gWidgetTheme.GetInt(WIDGET_TEXT_FG),
              bg    = gWidgetTheme.GetInt(WIDGET_TEXT_BG);
  char        c[10];
  TBCD        tbcd;

  // Integer part
  tbcd.FromUint32(TUint32(mSelectedValue));
  tbcd.ToString(c, EFalse);

  if (mIsFloat) {
    // Decimal separator
    char *p = &c[strlen(c)];
    *p++ = '.';

    // Decimal places
    tbcd.FromUint32(TUint32(mSelectedValue * mRange->precision) % mRange->precision);
    tbcd.ToString(&c[strlen(c)], EFalse);
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
