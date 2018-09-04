// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "Ruler.h"
#include <CryMath/Cry_Math.h>
#include "QtUtil.h"
#include "EditorStyleHelper.h"

#include <QPainter>

namespace DrawingPrimitives
{
enum
{
	RULER_MIN_PIXELS_PER_SEC = 3,	// PERSONAL VERIFY: 3 fine or something else? Double check how well timeline/ruler works too.
};

// pScreenRulerRange == Range in PIXELS
void CalculateTicks(uint size, TRange<CTimeValue> visibleRange, TRange<CTimeValue> rulerRange, int* pRulerPrecision, TRange<int32>* pScreenRulerRange, Ticks& ticksOut, const TRange<CTimeValue>* innerRange)
{
	ticksOut.clear();

	if (size == 0)
	{
		if (pRulerPrecision)
		{
			*pRulerPrecision = 0;
		}

		return;
	}

	const rTime pixelsPerUnit = visibleRange.Length() > 0 ? (mpfloat)size / visibleRange.Length() : 1;

	const CTimeValue startTime = rulerRange.start;
	const CTimeValue endTime = rulerRange.end;
	const CTimeValue totalDuration = endTime - startTime;

	const rTime ticksMinPower = log10(rTime((int)RULER_MIN_PIXELS_PER_SEC));
	const rTime ticksPowerDelta = ticksMinPower - log10(pixelsPerUnit);

	const int digitsAfterPoint = max(-int(ceil(ticksPowerDelta)) - 1, 0);
	if (pRulerPrecision)
	{
		*pRulerPrecision = digitsAfterPoint;
	}

	const CTimeValue scaleStep = pow(rTime(10), ceil(ticksPowerDelta)).conv<mpfloat>();
	const mpfloat scaleStepPixels = (scaleStep * pixelsPerUnit);
	const int numMarkers = int(totalDuration.GetSeconds() / scaleStep) + 1;

	const CTimeValue startTimeRound = int(startTime / scaleStep) * scaleStep;
	const int32 startOffsetMod      = int(startTime / scaleStep) % 10;
	const mpfloat scaleOffsetPixels   = ((startTime - startTimeRound) * pixelsPerUnit);

	const mpfloat startX = (rulerRange.start - visibleRange.start) * pixelsPerUnit;
	const mpfloat endX   = startX + (numMarkers - 1) * scaleStepPixels - scaleOffsetPixels;

	if (pScreenRulerRange)
	{
		*pScreenRulerRange = TRange<int32>((int32)startX, (int32)endX);
	}

	const int startLoop = std::max((int)((scaleOffsetPixels - startX) / scaleStepPixels) - 1, 0);
	const int endLoop   = std::min((int)((size + scaleOffsetPixels - startX) / scaleStepPixels) + 1, numMarkers);

	const int32 innerNumMarkers = innerRange ? int32((innerRange->end - innerRange->start) / scaleStep) : 0;
	const mpfloat innerBegX = innerRange ? (innerRange->start - visibleRange.start) * pixelsPerUnit - 1 : startX;
	const mpfloat innerEndX = innerRange ? innerBegX + innerNumMarkers * scaleStepPixels + 1 : endX;

	for (int i = startLoop; i < endLoop; ++i)
	{
		STick tick;

		const mpfloat xi = startX + i;
		const mpfloat x = xi * scaleStepPixels - scaleOffsetPixels;
		const CTimeValue value = startTimeRound + i * scaleStep;

		tick.m_bTenth = (startOffsetMod + i) % 10 != 0;
		tick.m_position = (int)x;
		tick.m_value = value;
		tick.m_bIsOuterTick = x<innerBegX || x> innerEndX;

		ticksOut.push_back(tick);
	}
}

void DrawTicks(const std::vector<STick>& ticks, QPainter& painter, const QPalette& palette, const STickOptions& options)
{
	QVector<QLine> innerTicks;
	innerTicks.reserve(ticks.size());
	QVector<QLine> outerTicks;
	outerTicks.reserve(ticks.size());

	const int32 height = options.m_rect.height();
	const int32 top = options.m_rect.top();
	const int32 lowY = top + height - options.m_markHeight;
	const int32 highY = top + height - options.m_markHeight / 2;

	for (const STick& tick : ticks)
	{
		const int x = tick.m_position + options.m_rect.left();
		const QLine line = QLine(QPoint(x, !tick.m_bTenth ? lowY : highY), QPoint(x, top + height));
		if (!tick.m_bIsOuterTick)
			innerTicks.append(line);
		else
			outerTicks.append(line);
	}

	painter.setPen(QPen(GetStyleHelper()->rulerInnerTicks()));
	painter.drawLines(innerTicks);

	painter.setPen(QPen(GetStyleHelper()->rulerOuterTicks()));
	painter.drawLines(outerTicks);
}

void DrawTicks(QPainter& painter, const QPalette& palette, const SRulerOptions& options)
{
	std::vector<STick> ticks;
	CalculateTicks(options.m_rect.width(), options.m_visibleRange, options.m_rulerRange, nullptr, nullptr, ticks);
	DrawTicks(ticks, painter, palette, options);
}

void DrawRuler(QPainter& painter, const SRulerOptions& options, int* pRulerPrecision)
{
	int rulerPrecision;
	TRange<int32> screenRulerRange;
	std::vector<STick> ticks;
	CalculateTicks(options.m_rect.width(), options.m_visibleRange, options.m_rulerRange, &rulerPrecision, &screenRulerRange, ticks, options.m_pInnerRange);

	if (pRulerPrecision)
	{
		*pRulerPrecision = rulerPrecision;
	}

	// We don't want shadows in the new flat design.
	/*if (options.m_shadowSize > 0)
	   {
	   QRect shadowRect = QRect(options.m_rect.left(), options.m_rect.height(), options.m_rect.width(), options.m_shadowSize);
	   QLinearGradient upperGradient(shadowRect.left(), shadowRect.top(), shadowRect.left(), shadowRect.bottom());
	   upperGradient.setColorAt(0.0f, QColor(0, 0, 0, 128));
	   upperGradient.setColorAt(1.0f, QColor(0, 0, 0, 0));
	   QBrush upperBrush(upperGradient);
	   painter.fillRect(shadowRect, upperBrush);
	   }*/

	painter.fillRect(options.m_rect, GetStyleHelper()->rulerBackground());
	if (options.m_drawBackgroundCallback)
		options.m_drawBackgroundCallback();

	const QColor innerTickColor = GetStyleHelper()->rulerInnerTicks();
	const QColor outerTickColor = GetStyleHelper()->rulerOuterTicks();

	QFont font;
	font.setPixelSize(10);
	painter.setFont(font);

	char format[16];
	cry_sprintf(format, "%%.%df", rulerPrecision);

	const int height = options.m_rect.height();
	const int top = options.m_rect.top();

	const int rulerYBase = top + height - options.m_ticksYOffset - 1;

	const QColor innerTickTextColor = GetStyleHelper()->rulerInnerTickText();
	const QColor outerTickTextColor = GetStyleHelper()->rulerOuterTickText();

	QString str;
	for (const STick& tick : ticks)
	{
		const int x = tick.m_position + options.m_rect.left();
		const CTimeValue value = tick.m_value;

		if (!tick.m_bIsOuterTick)
			painter.setPen(innerTickColor);
		else
			painter.setPen(outerTickColor);

		if (tick.m_bTenth)
		{
			painter.drawLine(QPoint(x, rulerYBase - options.m_markHeight / 2), QPoint(x, rulerYBase));
		}
		else
		{
			painter.drawLine(QPoint(x, rulerYBase - options.m_markHeight), QPoint(x, rulerYBase));

			if (!tick.m_bIsOuterTick)
				painter.setPen(innerTickTextColor);
			else
				painter.setPen(outerTickTextColor);

			str.sprintf(format, (float)value.GetSeconds());
			painter.drawText(QPoint(x + 2, rulerYBase - options.m_markHeight + 1), str);
		}
	}

	// Disabled. We don't want a fake 3D timeline in the new flat design.
	//painter.setPen(QPen(palette.color(QPalette::Dark)));
	//painter.drawLine(QPoint(options.m_rect.left(), 0), QPoint(options.m_rect.left(), options.m_rect.top() + height));
	//painter.drawLine(QPoint(options.m_rect.right(), 0), QPoint(options.m_rect.right(), options.m_rect.top() + height));
}

// NOTE: Converted to seconds or frame units depending on display mode.
mpfloat InUnits(const CTimeValue& animTime, const mpfloat& unit)
{
	return animTime.GetSeconds() / unit;
}

void CRuler::CalculateMarkers(TRange<int32>* pScreenRulerRange)
{
	// TODO: Refactor this function!

	if (m_options.rect.width() == 0)
	{
		return;
	}

	m_ticks.clear();

	const bool bIsFramesOrTimecode = m_options.timeUnit == SAnimData::EDisplayMode::Frames /* || m_options.timeUnit == SAnimTime::EDisplayMode::Timecode*/;
	const mpfloat unit = !bIsFramesOrTimecode ? 1 : m_options.secPerFrame.GetSeconds();
	const rTime fps = 1 / m_options.secPerFrame;

	const mpfloat startTick = InUnits(m_options.rulerRange.start, unit);
	const mpfloat endTick   = InUnits(m_options.rulerRange.end, unit);
	const mpfloat totalDuration = endTick - startTick;

	const mpfloat pixelsPerUnit = m_options.visibleRange.Length() > 0 ? m_options.rect.width() / InUnits(m_options.visibleRange.Length(), unit) : 1;

	const mpfloat ticksMinPower = log10(mpfloat(7));
	const mpfloat ticksPowerDelta = ticksMinPower - log10(pixelsPerUnit);

	m_decimalDigits = max(-int32(ceil(ticksPowerDelta)) - 1, 0);

	const mpfloat scaleStep = pow(mpfloat(10), ceil(ticksPowerDelta));
	const mpfloat scaleStepPixels = scaleStep * pixelsPerUnit;
	const int32 numMarkers = int32(totalDuration / scaleStep) + 1;

	const mpfloat startTimeRound = int32(startTick / scaleStep) * scaleStep;
	const int32   startOffsetMod = int32(startTick / scaleStep) % 10;
	const mpfloat scaleOffsetPixels = (startTick - startTimeRound) * pixelsPerUnit;

	const int32 startX = static_cast<int32>(InUnits(m_options.rulerRange.start - m_options.visibleRange.start, unit) * pixelsPerUnit);
	const int32 endX   = static_cast<int32>(startX + (numMarkers - 1) * scaleStepPixels - scaleOffsetPixels);

	if (pScreenRulerRange)
	{
		*pScreenRulerRange = TRange<int32>(startX, endX);
	}

	const int32 startLoop = std::max((int32)((scaleOffsetPixels - startX) / scaleStepPixels) - 1, 0);
	const int32 endLoop = std::min((int32)((m_options.rect.width() + scaleOffsetPixels - startX) / scaleStepPixels) + 1, numMarkers);

	const int32 innerNumMarkers = !m_options.innerRange.IsEmpty() ? int32(InUnits(m_options.innerRange.end - m_options.innerRange.start, unit) / scaleStep) : 0;
	const int32 innerBegX = !m_options.innerRange.IsEmpty() ? int32(InUnits(m_options.innerRange.start - m_options.visibleRange.start, unit) * pixelsPerUnit - 1) : startX;
	const mpfloat innerEndX = !m_options.innerRange.IsEmpty() ? innerBegX + innerNumMarkers * scaleStepPixels + 1 : endX;

	const mpfloat startFrameRound = startTimeRound * unit;
	const mpfloat frameScaleStep = scaleStep * unit;

	for (int i = startLoop; i < endLoop; ++i)
	{
		STick tick;

		const int32 xi = startX + i;
		const mpfloat x = xi * scaleStepPixels - scaleOffsetPixels;

		tick.position = (int32)x;
		if (!bIsFramesOrTimecode)
		{
			tick.bTenth = ((startOffsetMod + i) % 10) == 0;
			tick.value.SetSeconds(startTimeRound + i * scaleStep);
		}
		else
		{
			tick.bTenth = endLoop <= fps || ((startOffsetMod + i) % 10) == 0;
			if (/*m_options.timeUnit == SAnimTime::EDisplayMode::Frames && */ !tick.bTenth)
				continue;

			tick.value.SetSeconds(startFrameRound + i * frameScaleStep);
		}
		tick.bIsOuterTick = x<innerBegX || x> innerEndX;

		m_ticks.push_back(tick);
	}
}

void CRuler::Draw(QPainter& painter, const QPalette& palette)
{
	if (m_options.rect.width() == 0)
	{
		return;
	}

	painter.fillRect(m_options.rect, QtUtil::InterpolateColor(palette.color(QPalette::Button), palette.color(QPalette::Midlight), 0.25f));

	const QColor innerTickColor = GetStyleHelper()->rulerInnerTicks();
	const QColor outerTickColor = GetStyleHelper()->rulerOuterTicks();

	QFont font;
	font.setPixelSize(10);
	painter.setFont(font);

	char format[16];
	GenerateFormat(format);

	const int32 height = m_options.rect.height();
	const int32 top = m_options.rect.top();

	const int32 rulerYBase = top + height - m_options.ticksYOffset - 1;

	const QColor innerTickTextColor = GetStyleHelper()->rulerInnerTickText();
	const QColor outerTickTextColor = GetStyleHelper()->rulerOuterTickText();

	const int32 lowMarker = m_options.markHeight;
	const int32 highMarker = m_options.markHeight / 2;

	QString str;
	for (const STick& tick : m_ticks)
	{
		const int32 x = tick.position + m_options.rect.left();

		if (!tick.bIsOuterTick)
			painter.setPen(innerTickColor);
		else
			painter.setPen(outerTickColor);

		if (!tick.bTenth)
		{
			painter.drawLine(QPoint(x, rulerYBase - highMarker), QPoint(x, rulerYBase));
		}
		else
		{
			painter.drawLine(QPoint(x, rulerYBase - lowMarker), QPoint(x, rulerYBase));

			if (!tick.bIsOuterTick)
				painter.setPen(innerTickTextColor);
			else
				painter.setPen(outerTickTextColor);

			painter.drawText(QPoint(x + 2, rulerYBase - m_options.markHeight + 1), ToString(tick, str, format));
		}
	}
}

void CRuler::GenerateFormat(char* szFormatOut)
{
	switch (m_options.timeUnit)
	{
	default:
	case SAnimData::EDisplayMode::Time:
		sprintf(szFormatOut, "%%.%df", m_decimalDigits);
		return;
	case SAnimData::EDisplayMode::Frames:
	case SAnimData::EDisplayMode::Ticks:
		sprintf(szFormatOut, "%%d");
		return;
	case SAnimData::EDisplayMode::Timecode:
		{
			const rTime fps = 1 / m_options.secPerFrame;
			if (fps < 100)
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.2d");
			else if (fps < 1000)
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.3d");
			else
				sprintf(szFormatOut, "%%.2d:%%.2d:%%.4d");
		}
		return;
	}
}

QString& CRuler::ToString(const STick& tick, QString& strOut, const char* szFormat)
{
	CryFixedStringT<16> str;
	switch (m_options.timeUnit)
	{
	default:
	case SAnimData::EDisplayMode::Time:
		{
			str.Format(szFormat, (float)tick.value.GetSeconds());
		}
		break;
	case SAnimData::EDisplayMode::Frames:
		{
			int32 frame = int32(tick.value / m_options.secPerFrame);
			str.Format(szFormat, frame);
		}
		break;
	case SAnimData::EDisplayMode::Ticks:
		{
			str.Format(szFormat, int32(tick.value.GetSeconds() * SAnimData::numTicksPerSecond));
		}
		break;
	case SAnimData::EDisplayMode::Timecode:
		{
			int32 seconds = static_cast<int32>(tick.value.GetSeconds());
			int32 frames = int32((tick.value - seconds) / m_options.secPerFrame);
			int32 minutes = seconds / 60;
			seconds -= minutes * 60;

			str.Format(szFormat, minutes, seconds, frames);
		}
		break;
	}

	strOut = str.c_str();
	return strOut;
}
}

