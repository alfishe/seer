#include "SeerHexWidget.h"
#include "SeerUtl.h"
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QClipboard>
#include <QtGui/QTextCursor>
#include <QtCore/QSize>
#include <QtCore/QDebug>
#include <stdexcept>

#define byteArrayToType( data, order, precision, type ) \
        QDataStream stream( data ); \
        stream.setByteOrder( order ); \
        stream.setFloatingPointPrecision( precision ); \
        type t; \
        stream >> t; \
        return t;

qint8 toQInt8 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, qint8)
}

quint8 toQUInt8 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, quint8)
}

qint16 toQInt16 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, qint16)
}

quint16 toQUInt16 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, quint16)
}

qint32 toQInt32 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, qint32)
}

quint32 toQUInt32 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, quint32)
}

qint64 toQInt64 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, qint64)
}

quint64 toQUInt64 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, quint64)
}

float toQFloat32 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::SinglePrecision, float)
}

double toQFloat64 (const QByteArray& data, const QDataStream::ByteOrder order=QDataStream::BigEndian) {
    byteArrayToType(data, order, QDataStream::DoublePrecision, double)
}

SeerHexWidget::SeerHexWidget(QWidget* parent) : QWidget(parent), _pdata(NULL) {

    // Construct the UI.
    setupUi(this);

    // Setup the widgets
    QFont font;
    font.setFamily("monospace [Consolas]");
    font.setFixedPitch(true);
    font.setStyleHint(QFont::TypeWriter);

    plainTextEdit->setFont(font);
    plainTextEdit->setFocusPolicy(Qt::StrongFocus);
    plainTextEdit->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    _memoryMode    = SeerHexWidget::HexMemoryMode;
    _charMode      = SeerHexWidget::AsciiCharMode;
    _addressOffset = 0;
    _charWidth     = plainTextEdit->fontMetrics().horizontalAdvance(QLatin1Char('9'));
    _charHeight    = plainTextEdit->fontMetrics().height();
    _gapAddrHex    = 10; // Gap between address and hex fields.
    _gapHexAscii   = 16; // Gap between hex and ascii fields.

    setBytesPerLine(16);

    // Connect things.
    QObject::connect(plainTextEdit,                   &QPlainTextEdit::cursorPositionChanged,        this,  &SeerHexWidget::handleCursorPositionChanged);
    QObject::connect(showAsLittleEndianCheckBox,      &QCheckBox::clicked,                           this,  &SeerHexWidget::handleCursorPositionChanged);
    QObject::connect(showUnsignedFloatAsHexCheckBox,  &QCheckBox::clicked,                           this,  &SeerHexWidget::handleCursorPositionChanged);
    QObject::connect(this,                            &SeerHexWidget::byteOffsetChanged,             this,  &SeerHexWidget::handleByteOffsetChanged);
}

SeerHexWidget::~SeerHexWidget() {

    if (_pdata) {
        delete _pdata;
    }
}

void SeerHexWidget::setBytesPerLine (int count) {

    // 3 = 'a0 '
    // 4 = '007 '
    // 9 = '00011010 '
    // 4 = ' 10 '
    if (memoryMode() == SeerHexWidget::HexMemoryMode) {
        _hexCharsPerByte = 3;
    }else if (memoryMode() == SeerHexWidget::OctalMemoryMode) {
        _hexCharsPerByte = 4;
    }else if (memoryMode() == SeerHexWidget::BinaryMemoryMode) {
        _hexCharsPerByte = 9;
    }else if (memoryMode() == SeerHexWidget::DecimalMemoryMode) {
        _hexCharsPerByte = 4;
    }else{
        _hexCharsPerByte = 3;
    }

    _bytesPerLine    = count;
    _hexCharsPerLine = _bytesPerLine * _hexCharsPerByte - 1;
    _posAddr         = 0; // x position of address field.
    _posHex          = SeerHexWidget::HexFieldWidth * _charWidth + gapAddrHex(); // x position of hex field.
    _posAscii        = _posHex + hexCharsPerLine() * _charWidth + gapHexAscii(); // x position of ascii field.

    setMinimumWidth(_posAscii + (bytesPerLine() * _charWidth)); // x position after the ascii field.

    // Repaint the widget.
    create();
}

int SeerHexWidget::bytesPerLine () const {
    return _bytesPerLine;
}

int SeerHexWidget::hexCharsPerLine () const {
    return _hexCharsPerLine;
}

int SeerHexWidget::hexCharsPerByte () const {
    return _hexCharsPerByte;
}

int SeerHexWidget::nLines () const {
    return plainTextEdit->blockCount();
}

int SeerHexWidget::gapAddrHex () const {
    return _gapAddrHex;
}

int SeerHexWidget::gapHexAscii () const {
    return _gapHexAscii;
}

void SeerHexWidget::setAddressOffset (unsigned long offset) {

    _addressOffset = offset;

    // Repaint the widget.
    create();
}

unsigned long SeerHexWidget::addressOffset () const {
    return _addressOffset;
}

unsigned long SeerHexWidget::size () const {

    if (_pdata) {
        return _pdata->size();
    }

    return 0;
}

void SeerHexWidget::setMemoryMode (SeerHexWidget::MemoryMode memoryMode) {

    _memoryMode = memoryMode;

    // This repaints the widget with the new memory mode
    setBytesPerLine(bytesPerLine());
}

SeerHexWidget::MemoryMode SeerHexWidget::memoryMode () const {
    return _memoryMode;
}

QString SeerHexWidget::memoryModeString () const {

    if (memoryMode() == SeerHexWidget::HexMemoryMode) {
        return "hex";
    }else if (memoryMode() == SeerHexWidget::OctalMemoryMode) {
        return "octal";
    }else if (memoryMode() == SeerHexWidget::BinaryMemoryMode) {
        return "binary";
    }else if (memoryMode() == SeerHexWidget::DecimalMemoryMode) {
        return "decimal";
    }

    return "???";
}

void SeerHexWidget::setCharMode (SeerHexWidget::CharMode charMode) {

    _charMode = charMode;

    // Repaint the widget.
    create();
}

SeerHexWidget::CharMode SeerHexWidget::charMode() const {
    return _charMode;
}

QString SeerHexWidget::charModeString () const {

    if (charMode() == SeerHexWidget::AsciiCharMode) {
        return "ascii";
    }else if (charMode() == SeerHexWidget::EbcdicCharMode) {
        return "ebcdic";
    }

    return "???";
}

QTextDocument* SeerHexWidget::document () {

    return plainTextEdit->document();
}

QString SeerHexWidget::toPlainText () {

    return plainTextEdit->toPlainText();
}

void SeerHexWidget::setData(SeerHexWidget::DataStorage* pData) {

    if (_pdata) {
        delete _pdata;
    }

    _pdata     = pData;
    _cursorPos = 0;

    // Repaint the widget.
    create();
}

void SeerHexWidget::handleCursorPositionChanged () {

    // Get the current cursor position.
    QTextCursor cursor = plainTextEdit->textCursor();

    // Is it before the hex values? (address region)
    if (cursor.positionInBlock() < SeerHexWidget::HexFieldWidth) {
        emit byteOffsetChanged(-1);
        return;
    }

    // Is is after the hex values? (ascii/ebcdic region)
    if (cursor.positionInBlock() > SeerHexWidget::HexFieldWidth + hexCharsPerLine()) {
        emit byteOffsetChanged(-1);
        return;
    }

    //qDebug() << '(' << cursor.blockNumber() << ',' << cursor.positionInBlock() << ')';

    int line = cursor.blockNumber();
    int col  = cursor.positionInBlock() - SeerHexWidget::HexFieldWidth;
    int byte = (col / hexCharsPerByte()) + (line * bytesPerLine());

    //qDebug() << "(" << line << "," << col << ") =" << byte;

    emit byteOffsetChanged(byte);
}

void SeerHexWidget::handleByteOffsetChanged (int byte) {

    //qDebug() << byte;

    // Clear all fields.
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");
    lineEdit_7->setText("");
    lineEdit_8->setText("");
    lineEdit_9->setText("");
    lineEdit_10->setText("");

    if (byte < 0) {
        return;
    }

    // If there's no data, do nothing.
    if (!_pdata) {
        return;
    }

    // Set the endian default.
    QDataStream::ByteOrder byteOrder = QDataStream::BigEndian;

    if (showAsLittleEndianCheckBox->isChecked()) {
        byteOrder = QDataStream::LittleEndian;
    }

    // Set the 'AsHex' default.
    bool unsignedAndFloatAsHex = false;

    if (showUnsignedFloatAsHexCheckBox->isChecked()) {
        unsignedAndFloatAsHex = true;
    }

    // Go through each one and display the value.
    {
        QByteArray arr = _pdata->getData(byte, sizeof(char));                       // Extract a bytearray from the data for the size of the value we are after.
        if (arr.size() == sizeof(char)) {                                           // If not the right size, skip it. Near the end of the data.
            lineEdit_1->setText(QString::number(toQInt8(arr, byteOrder)));          // Fill in the signed value.
            if (unsignedAndFloatAsHex) {                                            // Show unsigned value as hex?
                if (byteOrder == QDataStream::LittleEndian) {                       // Swap bytes to handle endianess.
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_2->setText("0x"+QString(arr.toHex()));                     // Print value as hex.
            }else{
                lineEdit_2->setText(QString::number(toQUInt8(arr, byteOrder)));     // Print value as a value.
            }
        }
    }

    {
        QByteArray arr = _pdata->getData(byte, sizeof(short));
        if (arr.size() == sizeof(short)) {
            lineEdit_3->setText(QString::number(toQInt16(arr, byteOrder)));
            if (unsignedAndFloatAsHex) {
                if (byteOrder == QDataStream::LittleEndian) {
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_4->setText("0x"+QString(arr.toHex()));
            }else{
                lineEdit_4->setText(QString::number(toQUInt16(arr, byteOrder)));
            }
        }
    }

    {
        QByteArray arr = _pdata->getData(byte, sizeof(float));
        if (arr.size() == sizeof(float)) {
            if (unsignedAndFloatAsHex) {
                if (byteOrder == QDataStream::LittleEndian) {
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_5->setText("0x"+QString(arr.toHex()));
            }else{
                lineEdit_5->setText(QString::number(toQFloat32(arr, byteOrder)));
            }
        }
    }

    {
        QByteArray arr = _pdata->getData(byte, sizeof(int));
        if (arr.size() == sizeof(int)) {
            lineEdit_6->setText(QString::number(toQInt32(arr, byteOrder)));
            if (unsignedAndFloatAsHex) {
                if (byteOrder == QDataStream::LittleEndian) {
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_7->setText("0x"+QString(arr.toHex()));
            }else{
                lineEdit_7->setText(QString::number(toQUInt32(arr, byteOrder)));
            }
        }
    }

    {
        QByteArray arr = _pdata->getData(byte, sizeof(long int));
        if (arr.size() == sizeof(long int)) {
            lineEdit_8->setText(QString::number(toQInt64(arr, byteOrder)));
            if (unsignedAndFloatAsHex) {
                if (byteOrder == QDataStream::LittleEndian) {
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_9->setText("0x"+QString(arr.toHex()));
            }else{
                lineEdit_9->setText(QString::number(toQUInt64(arr, byteOrder)));
            }
        }
    }

    {
        QByteArray arr = _pdata->getData(byte, sizeof(double));
        if (arr.size() == sizeof(double)) {
            if (unsignedAndFloatAsHex) {
                if (byteOrder == QDataStream::LittleEndian) {
                    std::reverse(arr.begin(), arr.end());
                }
                lineEdit_10->setText("0x"+QString(arr.toHex()));
            }else{
                lineEdit_10->setText(QString::number(toQFloat64(arr, byteOrder)));
            }
        }
    }

}

void SeerHexWidget::create () {

    // Clear the current document. We're going to recreate it.
    plainTextEdit->clear();

    // If there's no data, do nothing.
    if (!_pdata) {
        return;
    }

    // Set text formats.
    QTextCharFormat defaultFormat = plainTextEdit->currentCharFormat();
    QTextCharFormat grayFormat    = defaultFormat;
    grayFormat.setBackground(QBrush(Qt::lightGray));

    // Get a cursor
    QTextCursor cursor(plainTextEdit->textCursor());

    cursor.movePosition(QTextCursor::Start);

    // Go through the data, one byte at a time.
    for (int i=0; i<_pdata->size(); i+=bytesPerLine()) {

        QByteArray data = _pdata->getData(i, bytesPerLine());

        // Place a new hex address on the left side.
        if (i % bytesPerLine() == 0) {

            QString address = QString("%1").arg(i + addressOffset(), SeerHexWidget::HexFieldWidth, 16, QChar('0'));
            QString spacer(" ");

            // Write adress to document.
            cursor.insertText (address, grayFormat);

            // Write spacer to document.
            cursor.insertText (spacer, defaultFormat);
        }

        // Print N bytes in their datatype value.
        int b = 0;

        for (b=0; b<bytesPerLine() && i+b < _pdata->size(); b++) {

            unsigned char ch = data[b];

            QString val;

            if (memoryMode() == SeerHexWidget::HexMemoryMode) {
                val = QString("%1").arg(ushort(ch), int(2), int(16), QChar('0'));
            }else if (memoryMode() == SeerHexWidget::OctalMemoryMode) {
                val = QString("%1").arg(ushort(ch), int(3), int(8), QChar('0'));
            }else if (memoryMode() == SeerHexWidget::BinaryMemoryMode) {
                val = QString("%1").arg(ushort(ch), int(8), int(2), QChar('0'));
            }else if (memoryMode() == SeerHexWidget::DecimalMemoryMode) {
                val = QString("%1").arg(ushort(ch), int(3), int(10), QChar(' '));
            }else{
                val = "??";
            }

            QString spacer(" ");

            // Write memory value to document
            cursor.insertText (val, defaultFormat);

            // Write spacer to document.
            cursor.insertText (spacer, defaultFormat);
        }

        // Print N odd remainder bytes.
        for (; b<bytesPerLine(); b++) {

            QString val;

            if (memoryMode() == SeerHexWidget::HexMemoryMode) {
                val = "  ";
            }else if (memoryMode() == SeerHexWidget::OctalMemoryMode) {
                val = "   ";
            }else if (memoryMode() == SeerHexWidget::BinaryMemoryMode) {
                val = "        ";
            }else if (memoryMode() == SeerHexWidget::DecimalMemoryMode) {
                val = "   ";
            }else{
                val = "??";
            }

            QString spacer(" ");

            // Write memory value to document
            cursor.insertText (val, defaultFormat);

            // Write spacer to document.
            cursor.insertText (spacer, defaultFormat);
        }

        // Print vertical line.
        QString spacer("| ");

        // Write spacer to document.
        cursor.insertText (spacer, defaultFormat);

        // Print N bytes in their char value.
        if (charMode() == SeerHexWidget::AsciiCharMode) {
            for (int b=0; b<bytesPerLine() && i+b < _pdata->size(); b++) {

                unsigned char ch = Seer::ucharToAscii( data[b] );

                QString val(ch);

                // Write display character to document.
                cursor.insertText (val, defaultFormat);
            }

        }else if (charMode() == SeerHexWidget::EbcdicCharMode) {
            for (int b=0; b<bytesPerLine() && i+b < _pdata->size(); b++) {

                unsigned char ch = Seer::ebcdicToAscii( data[b] );

                QString val(ch);

                // Write display character to document.
                cursor.insertText (val, defaultFormat);
            }

        }else{
            // Don't print anything.
        }

        // Write eol to document.
        QString eol("\n");

        cursor.insertText (eol, defaultFormat);
    }
}

SeerHexWidget::DataStorageArray::DataStorageArray(const QByteArray& arr) {
    _data = arr;
}

QByteArray SeerHexWidget::DataStorageArray::getData(int position, int length) {
    return _data.mid(position, length);
}

int SeerHexWidget::DataStorageArray::size() {
    return _data.count();
}

