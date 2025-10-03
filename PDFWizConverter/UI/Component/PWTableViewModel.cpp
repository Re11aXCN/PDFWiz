#include "PWTableViewModel.h"
#include <QIcon>
#include <QPixmap>
PWTableViewModel::PWTableViewModel(QObject* parent)
    : QAbstractTableModel{ parent }
{
    _pCheckIconList.reserve(5);
    _pFileTypeIconList.reserve((qsizetype)WizConverter::Module::Enums::FileType::__END);
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Checked.svg").scaled(QSize{ 23, 23 }, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Partially_Checked.svg").scaled(QSize{ 23, 23 }, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked.svg").scaled(QSize{ 23, 23 }, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked_Hover.svg").scaled(QSize{ 23, 23 }, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _pCheckIconList.append(QIcon(QPixmap(":/Resource/Image/Button/Checkbox_Unchecked_Press.svg").scaled(QSize{ 23, 23 }, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_CAD.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_CAJ.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_EPUB.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_EXCEL.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_HTML.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_IMAGE.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_MARKDOWN.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_OFD.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_PDF.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_POWERPOINT.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_TXT.png")));
    _pFileTypeIconList.append(QIcon(QPixmap(":/Resource/Image/Button/FileType_WORD.png")));

    _pHeader = { "", "文件名", "总页数", "转换页面范围", "输出格式", "状态", "操作", "删除" };
}

PWTableViewModel::~PWTableViewModel()
{
}

QVariant PWTableViewModel::data(const QModelIndex& index, int role) const
{
    return QVariant();
}


int PWTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _pDataList.size();
}

int PWTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _pHeader.size();
}

QVariant PWTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (role)
        {
        case Qt::DisplayRole: {
            if (section == 1 && rowCount() != 0)
            {
                return QString("全选      已选中(%1/%2)").arg(1).arg(1);
            }
            else if (section == columnCount() - 1)
            {
                return QVariant();
            }
            return _pHeader.at(section);
        } 
        case Qt::DecorationRole: {
            if (section == 0) {

            }
        }
        case Qt::TextAlignmentRole: {
            if(section == 1) return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }
        default: break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}


bool PWTableViewModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    return false;
}