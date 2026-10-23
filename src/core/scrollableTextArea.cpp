#include "scrollableTextArea.h"
#define _scrollBuffer tft
ScrollableTextArea::ScrollableTextArea(const String &title)
    : firstVisibleLine{0}, _redraw{true}, _title(title), _fontSize(FP), _startX(BORDER_PAD_X),
      _startY(BORDER_PAD_Y), _indentWrappedLines(false), _width(tftWidth - 2 * BORDER_PAD_X),
      _height(tftHeight - 4 - BORDER_PAD_X - BORDER_PAD_Y) {
    drawMainBorder();

    if (!_title.isEmpty()) {
        printTitle(_title);
        _startY = tft.getCursorY();
        _height -= (_startY - BORDER_PAD_Y);
    }

    setup();
}

ScrollableTextArea::ScrollableTextArea(
    uint8_t fontSize, int16_t startX, int16_t startY, int32_t width, int32_t height, bool drawBorders,
    bool indentWrappedLines
)
    : firstVisibleLine{0}, _redraw{true}, _title(""), _fontSize(fontSize), _startX(startX), _startY(startY),
      _width(width), _height(height), _indentWrappedLines(indentWrappedLines) {
    if (drawBorders) { drawMainBorder(); }
    setup();
}

ScrollableTextArea::~ScrollableTextArea() {
    // We don't use Sprites for big things, unfortunetly theres no much RAM in all devices
}

void ScrollableTextArea::setup() {
    _scrollBuffer.setTextColor(bruceConfig.priColor);
    _scrollBuffer.setTextSize(_fontSize);
    _scrollBuffer.fillRect(_startX, _startY, _width, _height, bruceConfig.bgColor);

    _maxCharactersPerLine = std::max<int>(2, _width / std::max<int>(1, _scrollBuffer.textWidth("w", _fontSize)));
    _pixelsPerLine = _scrollBuffer.fontHeight() + 2;
    _maxVisibleLines = std::max<int>(1, _height / _pixelsPerLine);
}

void ScrollableTextArea::scrollUp() {
    if (firstVisibleLine) {
        firstVisibleLine--;
        _redraw = true;
    }
}

void ScrollableTextArea::scrollDown() {
    const size_t maxFirst = linesBuffer.size() > _maxVisibleLines ? linesBuffer.size()-_maxVisibleLines : 0;
    if (firstVisibleLine < maxFirst) { ++firstVisibleLine; _redraw=true; }
}

void ScrollableTextArea::scrollToLine(size_t lineNumber) {
    if (linesBuffer.empty()) return; // Ensure there's content to scroll

    const size_t maxFirst = linesBuffer.size() > _maxVisibleLines ? linesBuffer.size()-_maxVisibleLines : 0;
    firstVisibleLine = std::min(lineNumber,maxFirst);
    _redraw=true;
}

String ScrollableTextArea::getLine(size_t lineNumber) {
    return lineNumber < linesBuffer.size() ? linesBuffer[lineNumber] : String();
}

size_t ScrollableTextArea::getMaxLines() { return linesBuffer.size(); }

void ScrollableTextArea::show(bool force) {
    draw(force);

    while (check(SelPress)) {
        update(force);
        yield();
    }
    while (!check(SelPress) && !check(EscPress)) {
        update(force);
        yield();
    }
}

uint32_t ScrollableTextArea::getMaxVisibleTextLength() { return _maxVisibleLines * _maxCharactersPerLine; }

void ScrollableTextArea::update(bool force) {
#ifdef HAS_ENCODER
    int32_t rotarySteps = drainRotarySteps();
    if (rotarySteps != 0) {
        check(PrevPress);
        check(NextPress);
        check(UpPress);
        check(DownPress);
        while (rotarySteps > 0) {
            scrollUp();
            rotarySteps--;
        }
        while (rotarySteps < 0) {
            scrollDown();
            rotarySteps++;
        }
        vTaskDelay(4 / portTICK_PERIOD_MS);
        PrevPress = false;
        NextPress = false;
        UpPress = false;
        DownPress = false;
    }
#endif

    if (check(PrevPress) || check(UpPress)) scrollUp();
    else if (check(NextPress) || check(DownPress)) scrollDown();

    draw(force);
}

void ScrollableTextArea::fromFile(File file) {
    linesBuffer.clear();
#if WS28B_TOUCH_PRO
    String line;size_t bytes=0;
    while(file.available()&&bytes<32768&&linesBuffer.size()<512){
        char c=char(file.read());++bytes;
        if(c=='\n'){addLine(line);line="";}
        else {if(line.length()>=512&&(uint8_t(c)&0xC0)!=0x80){addLine(line);line="";}line+=c;}
    }
    if(line.length())addLine(line);
    if(file.available())addLine("[view truncated: memory limit]");
#else
    while (file.available()) addLine(file.readStringUntil('\n'));
#endif

    draw(true);
}

void ScrollableTextArea::clear() {
    firstVisibleLine = 0;
    linesBuffer.clear();
    _redraw=true;
}

void ScrollableTextArea::fromString(const String &text) {
    clear();
    int startIdx = 0;
    int endIdx = 0;

    while (endIdx < text.length()) {
        if (text[endIdx] == '\n') {
            addLine(text.substring(startIdx, endIdx));
            startIdx = endIdx + 1;
        }

        endIdx++;
    }

    // Add the last line if there's remaining text (text does not ends with \n)
    if (startIdx < text.length()) { addLine(text.substring(startIdx, endIdx)); }
}

// for devices it will act as a scrollable text area
void ScrollableTextArea::addLine(const String &text) {
#if WS28B_TOUCH_PRO
    if(linesBuffer.size()>=512) {linesBuffer.back()="[view truncated: 512 rows]";_redraw=true;return; }
#endif
    if (text.isEmpty()) {
        linesBuffer.emplace_back("");
        _redraw=true;
        return;
    }

    String buff;
    size_t start = 0;
    bool firstLine = true;

    // Automatically split into multiple lines
    while (start < text.length()) {
#if WS28B_TOUCH_PRO
        if(linesBuffer.size()>=512) {linesBuffer.back()="[view truncated: 512 rows]";_redraw=true;return;}
#endif
        size_t len = _maxCharactersPerLine;
#if WS28B_TOUCH_PRO
        size_t end=start, cells=0;
        while(end<text.length() && cells<len-(_indentWrappedLines&&!firstLine ? 1:0)) {
            ++end;
            while(end<text.length() && (uint8_t(text[end])&0xC0)==0x80) ++end;
            ++cells;
        }
        buff=(!firstLine&&_indentWrappedLines ? String(" ") : String())+text.substring(start,end);
        start=end;
#else

        if (!firstLine && _indentWrappedLines) {
            buff = " " + text.substring(start, start + len - 1); // Reduce length for space
            start += len - 1;
        } else {
            buff = text.substring(start, start + len);
            start += len;
        }
#endif
        if (buff.endsWith("\r")) buff.remove(buff.length() - 1);

        linesBuffer.emplace_back(buff);
        firstLine = false;
    }

    _redraw = true;
}

void ScrollableTextArea::draw(bool force) {
    if (!_redraw && !force) return;

    _scrollBuffer.fillRect(_startX, _startY, _width, _height, bruceConfig.bgColor);
    _scrollBuffer.setTextColor(bruceConfig.priColor);
    uint8_t _fSize = tft.getTextSize();
    tft.setTextSize(_fontSize);

    firstVisibleLine=std::min(firstVisibleLine, linesBuffer.size());
    uint16_t yOffset = 0;
    size_t lines = 0;

    // if there is text above
    if (firstVisibleLine) {
        _scrollBuffer.drawString("...", 0 + _startX, _startY + yOffset);
        yOffset += _pixelsPerLine;
        lines++;
    }

    int32_t tmpHeight = _height;
    // if there is text below
    if (linesBuffer.size() - firstVisibleLine >= _maxVisibleLines) {
        _scrollBuffer.drawString("...", 0 + _startX, _startY + _height - _pixelsPerLine);
        tmpHeight -= _pixelsPerLine;
        lines++;
    }

    size_t idx{firstVisibleLine};
    while (yOffset < tmpHeight && lines < _maxVisibleLines && idx < linesBuffer.size()) {
        _scrollBuffer.drawString(linesBuffer[idx], 0 + _startX, _startY + yOffset);
        yOffset += _pixelsPerLine;
        lines++;
        idx++;
    }

    lastVisibleLine = firstVisibleLine + lines;
    tft.setTextSize(_fSize);

    _redraw = false;
}
