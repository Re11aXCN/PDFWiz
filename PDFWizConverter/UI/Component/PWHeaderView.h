#pragma once

#include <NXProperty.h>
#include <QHeaderView>

class PWHeaderView : public QHeaderView
{
	Q_OBJECT
	Q_PRIVATE_CREATE_D(bool, IsClearRectHovered)
public:
	explicit PWHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
	virtual ~PWHeaderView() override = default;

Q_SIGNALS:
	Q_SIGNAL void selectAllRows();
	Q_SIGNAL void removeSelectedRows();
protected:
	virtual void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
