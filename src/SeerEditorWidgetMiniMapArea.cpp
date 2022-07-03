#include "SeerEditorWidget.h"
#include <QtGui/QTextCursor>
#include <QtCore/QDebug>

SeerEditorWidgetSourceMiniMapArea::SeerEditorWidgetSourceMiniMapArea(SeerEditorWidgetSourceArea* editorWidget) : QWidget(editorWidget) {
    _editorWidget = editorWidget;
}

QSize SeerEditorWidgetSourceMiniMapArea::sizeHint () const {
    return QSize(_editorWidget->miniMapAreaWidth(), 0);
}

void SeerEditorWidgetSourceMiniMapArea::paintEvent (QPaintEvent* event) {
    _editorWidget->miniMapAreaPaintEvent(event);
}

void SeerEditorWidgetSourceMiniMapArea::mouseDoubleClickEvent (QMouseEvent* event) {

    QTextCursor  cursor = _editorWidget->cursorForPosition(event->pos());

    qDebug() << cursor.blockNumber()+1;

    QWidget::mouseDoubleClickEvent(event);
}

void SeerEditorWidgetSourceMiniMapArea::mouseMoveEvent (QMouseEvent* event) {

    QTextCursor  cursor = _editorWidget->cursorForPosition(event->pos());

    qDebug() << cursor.blockNumber()+1;

    QWidget::mouseMoveEvent(event);
}

void SeerEditorWidgetSourceMiniMapArea::mousePressEvent (QMouseEvent* event) {

    QTextCursor  cursor = _editorWidget->cursorForPosition(event->pos());

    qDebug() << cursor.blockNumber()+1;

    QWidget::mousePressEvent(event);
}

void SeerEditorWidgetSourceMiniMapArea::mouseReleaseEvent (QMouseEvent* event) {

    QTextCursor  cursor = _editorWidget->cursorForPosition(event->pos());

    qDebug() << cursor.blockNumber()+1;

    QWidget::mouseReleaseEvent(event);
}

