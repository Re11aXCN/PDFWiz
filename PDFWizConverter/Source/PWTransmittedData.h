#pragma once

#include <QString>
#include <QVector>

namespace WizConverter::Module::TransmittedData {

struct Base {
    QString OutputDir{ "" };
    bool IsAutoOpenDirectory{ true };
    virtual ~Base() {}
};

struct Deskew {
    bool IsUseAngleFilter{ true };
    bool IsConsider90{ true };
    int NumPeaks{ 11 };
    double Sigma{ 5.0 };
    double FilterMinAngle{ 0.0 };
    double FilterMaxAngle{ 90.0 };
    double MinDeviation{ 0.9 };
    virtual ~Deskew() {}
};

struct DocumentTranslate : public Base {
    bool IsKeepStyle{ false };
    QString TranslateTargetLanguage{ "英语" };
};

struct Watermark : public Base {
    /**/    bool IsRenderBackground{ true };
    /**/    bool IsTextRenderAsImage{ true };
    bool IsAutoAverageLayout{ true };
    /**/    int Width{ 100 };
    /**/    int Height{ 100 };
    int WaterMarkXIndent{ -10 };
    int WaterMarkYIndent{ -10 };
    int Columns{ 8 };
    int Rows{ 5 };
    int ColumnSpace{ 150 };
    int RowSpace{ 150 };
    int RotationalAngle{ 45 };
    /**/    int FontSize{ 16 };
    int FontStyle{ 0 }; // 0 regular 1 bold 2 italic
    /**/   long long ColorARGB{ 0x1B000000 };
    double WaterMarkImageOpacity{ 1.0 };
    /**/    double WaterMarkImageZoom{ 1.0 };
    /**/   QString FontName{ "SimSun" };
    QString WaterMarkText{ "PDFWiz" };
    /**/    QString WaterMarkImagePath{ "" };
    Qt::Alignment HorizontalAlignment{ Qt::AlignAbsolute };
    Qt::Alignment HerticalAlignment{ Qt::AlignAbsolute };
    virtual ~Watermark() {}
};

/*===================================================================*/
#pragma region PDFToWord
struct PDFToWord : public Base {
    bool IsOCREnabled{ false };
    bool IsSymbolRecognitionEnabled{ false };
    int RecognitionMode{ 0 };
    int WordFormat{ 0 };    // 0docx 1doc 2rtf 3md
};

struct PDFToExcel : public Base {
    bool IsInsertBlankColumnAtFirst{ false };
    bool IsMergeAllPages{ false };
    bool IsUniformWorkSheets{ false };
    int ExcelFormat{ 1 }; // 0xml xlsx csv xlsm ods
};

struct PDFToPowerpoint : public Base {
    bool IsOCREnabled{ false };
    bool IsSeparationImageAndGraphic{ false };
    bool IsRecognizeAllPageAsImage{ false };
};

struct PDFToImage : public Base {
    bool IsExportEachPageAsImage{ true };
    bool IsTransparentBackground{ false };
    bool IsUseDithering{ false };
    int DitherType{ 8 };
    int Width{ 100 };
    int Height{ 100 };
    int ImageDpi{ 300 };
    int MaxColor{ 256 };
    int TimeInterval{ 100 };
    int ImageFormat{ 0 }; // 0JEPG 1JPG 2PNG 3BMP 4TIFF 5TIF 6GIF 7SVG 
    double DitherScale{ 1.0 };
};

struct PDFToTxt : public Base {
    int TextInputMode{ 2 };
};

struct PDFToCad : public Base {
    int CadFormat{ 0 }; // 0dwg 1dxf
};

struct PDFToEpub : public Base {
    bool IsUsePdfEngine{ false };
    int ImageDpi{ 300 };
    int ContentSaveMode{ 0 };
    QString CoverImageFilePath{ "" };
    QString Author{ "" };
};

struct PDFToHtml : public Base {
    bool IsRenderTextAsImage{ false };
    QString FontName{ "SimSun" };
};

struct PDFToMarkdown : public Base {
};
#pragma endregion PDFToWord

/*===================================================================*/
#pragma region WordToPDF
struct WordToPDF : public Base {
    bool IsUseOfficeToPdf{ false };
    bool IsRetainDocumentMarkup{ false };
    bool IsEmbedFont{ false };
    bool IsUpdateField{ false };
    bool IsDeleteTempFolder{ true };
    int PdfComplianceOption{ 1 };
    int MswVersion{ 7 };
    int PdfColorMode{ 0 };
};

struct ExcelToPDF : public Base {
    enum PaperSizeType {
        Custom = 0,
        PaperLetter = 1,
        PaperLetterSmall = 2,
        PaperTabloid = 3,
        PaperLedger = 4,
        PaperLegal = 5,
        PaperStatement = 6,
        PaperExecutive = 7,
        PaperA3 = 8,
        PaperA4 = 9,
        PaperA4Small = 10,
        PaperA5 = 11,
        PaperB4 = 12,
        PaperB5 = 13,
        PaperFolio = 14,
        PaperQuarto = 15,
        Paper10x14 = 16,
        Paper11x17 = 17,
        PaperNote = 18,
        PaperCSheet = 24,
        PaperDSheet = 25,
        PaperESheet = 26,
        Paper9x11 = 44,
        Paper10x11 = 45,
        Paper15x11 = 46,
        PaperLetterPlus = 59,
        PaperA4Plus = 60,
        PaperA2 = 66,
        PaperA6 = 70,
        Paper12x11 = 90,
        PaperB3 = 300,
        PaperBusinessCard = 301,
    };
    bool IsExportEachSheet{ false };
    bool IsDividingMultiplePages{ false };
    bool IsPrintGridlines{ false };
    int PdfComplianceOption{ 0 };
    int PdfCompressionCore{ 0 };
    int PaperSizeType{ 0 };
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
};

struct PowerpointToPDF : public Base {
    bool IsUseOfficeToPdf{ false };
    bool IsEmbedFont{ false };
    bool IsShowHiddenSlides{ false };
    bool IsDrawSlidesFrame{ false };
    int ImageDpi{ 300 };
    int PdfComplianceOption{ 2 };
    int NotesPosition{ 0 };
};

struct ImageToPDF : public Base {
    bool IsMergeAllImagesToOnePdf{ false };
    bool IsOptimizePdf{ false };
    bool IsCompressImages{ false };
    bool IsLandscape{ false };  // 默认应该是Portrait 纵向排版，isLandscape = true 则横向排版
    bool IsStretchImageHAdaptePageH{ false };
    bool IsStretchImageVAdaptePageV{ false };
    int ImageEncoding{ 0 };
    int ImageCompressionVersion{ 0 };
    int PageSizeType{ 0 }; // 0Auto 1Max
    int ImageQuality{ 100 };
    int ImageDpi{ 300 };
    double ImageZoom{ 1.0 };
    Qt::Alignment AlignmentType{ Qt::AlignLeft | Qt::AlignTop };
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
};

struct TxtToPDF : public Base {
    bool RemoveXmlTags{ false };
    int FontStyle{ 0 };
    int ParagraphAlignment{ 0 };
    int PageSizeType{ 4 };
    long long ColorARGB{ 0xFF000000 };
    double FontSize{ 12.5 };
    double SpaceBefore{ 0 };
    double SpaceAfter{ 0 };
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
    QString FontName{ "SimSun" };
};

struct CadToPDF : public Base {

};

struct EpubToPDF : public Base {
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
    QString CustomFontPath{ "" };
};

struct HtmlToPDF : public Base {
    bool IsCustomURL{ false };
    bool IsLandscape{ false };
    bool IsRenderToSinglePage{ false };
    bool IsUseCss{ false };
    int ConvertedOption{ 0 };
    int PageSizeType{ 4 };
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
    QString ChromiumPath{ "" };
};

struct MarkdownToPDF : public Base {
    bool IsLandscape{ false };
    bool IsDeleteHtmlTempDir{ false };
    bool IsUseCss{ false };
    int PageSizeType{ 4 };
    double MarginTop{ 0 };
    double MarginBottom{ 0 };
    double MarginLeft{ 0 };
    double MarginRight{ 0 };
    QString ChromiumPath{ "" };
};
#pragma endregion WordToPDF

/*=====================================================================*/
#pragma region PDFAction
struct PDFSplit : public Base {
    bool IsNeedCrossPage{ false };
    bool IsRemoveCrossPageExcess{ false };
    bool IsCompressPDF{ false };
    int StartPageIndex{ 1 };
    int FixedPageNumber{ 1 };
    int SplitOption{ 0 };
};

struct PDFMerge : public Base {
    int PageSizeType{ 1 };   // 0Auto 1Max
    QString OutputFileName{ "" };
};

struct PDFCompress : public Base {
    bool IsUnembedFont{ true };
    bool IsCompressImage{ false };
    bool IsRemoveAnnotation{ false };
    bool IsRemoveUnusedObject{ true };
    bool IsRemoveUnusedStream{ true };
    bool IsLinkDuplicateStream{ true };
    int ImageQuality{ 100 };
    int ImageDpi{ 300 };
    int ImageEncoding{ 0 };
    int ImageCompressionVersion{ 0 };
};

struct PDFPageRenderAsImage : public Base {
    int ImageFormat{ 0 };// 0 JPEG 2 PNG
    int ImageDpi{ 300 };
};

struct PDFPageExtract : public Base {
    bool IsExtractPage{ true };
    bool IsExtractImage{ false };
    bool IsExtractText{ false };
    bool IsMergeText{ false };
    bool IsContainPageNumbers{ true };
    bool IsNeedOCR{ false };
    bool DoAngle{ false };
    bool MostAngle{ false };
    bool ImgDeskew{ false };
    int ImageFormat{ 0 }; // 0JEPG 1JPG 2PNG 3BMP 4TIFF 5TIF
    int Padding = 50;
    int ImgResize = 1024;
    int NumThread = 4;
    /*
        ChineseSimplified,  // 简体中文 0
        ChineseTraditional, // 繁體中文 1
        English,            // 英文     2
        Japanese,           // 日本語   3
        Korean,             // 한국어   4
        Russian             // Русский
    */
    int Language = 0;
    long long ColorARGB{ 0x00FFFFFF };
    double BoxScoreThresh = 0.5;
    double BoxThresh = 0.3;
    double UnClipRatio = 1.6;
};

struct PDFPageDelete : public Base {
    bool IsRemovePage{ true };
    bool IsRemoveImage{ false };
    bool IsRemoveText{ false };
    bool IsAddBlankPage{ true };
};

struct PDFCrypto : public Base {
    bool IsExportNewFile{ true };
    bool IsDecrypt{ false };
    QString UserPassword{ "" };
    QString OwnerPassword{ "" };
};

struct PDFWatermark : public Watermark {

};
#pragma endregion PDFAction

/*=====================================================================*/
#pragma region ImageAction
struct ImageResize : public Base {
    bool IsCustomSize{ false };
    int NewImageWidth{ 1920 };
    int NewImageHeight{ 1080 };
    double Zoom{ 1.0 };
};

struct ImageCompress : public Base {
    bool IsOptimizePngTransparency{ false };
    int PngCompressionLevel{ 6 };
    int JpgeCompressionQuality{ 90 };
    int GifDifference{ 80 };
    int PaletteEntriesCount{ 256 };
    int TiffCompressionType{ 1 };
    int BmpCompressionType{ 1 };
    int PngColorType{ 1 };
    int PngFilterType{ 5 };
};

struct ImageTransfer : public Base {
    bool IsMergeImages{ false };
    bool IsUseDithering{ false };
    bool IsHorizontalStretch{ false };
    bool IsVerticalStretch{ false };
    int ImageFormat{ 0 }; // 0JEPG 1JPG 2PNG 3BMP 4TIFF 5TIF 6GIF
    int DitherType{ 8 };
    int Width{ 100 };
    int Height{ 100 };
    int MaxColor{ 256 };
    int TimeInterval{ 100 };
    double DitherScale{ 1.0 };
    QString LongImageName{ "" };
};

struct ImageWatermark : public Watermark {
};

struct ImageOCR : public Base {
    bool IsOutputSingleFile{ false };
    bool IsKeepBlankFileInfo{ false };
    bool IsContainImgNameInfo{ false };
    bool DoAngle{ false };
    bool MostAngle{ false };
    bool ImgDeskew{ false };
    int Padding = 50;
    int ImgResize = 1024;
    int NumThread = 4;
    int Language = 0;
    double BoxScoreThresh = 0.5;
    double BoxThresh = 0.3;
    double UnClipRatio = 1.6;
    int OutputFormat{ 0 };
};

struct ImageRotateCropFlip : public Base, public Deskew {
    int RotateOption{ 1 };
    int CropOption{ 0 };
    int DeskewOption{ 0 };
    int Width{ 100 };
    int Height{ 100 };
    int XIndent{ 0 };
    int YIndent{ 0 };
    int ImageRotationalAngle{ 0 };
    int RotateMode{ 0 };
    int FlipMode{ 0 };
    int PositionMode{ 0 }; //0 Custom 1 TL 2 TC 3 LC 4 C
    long long ColorARGB{ 0x00FFFFFF };
};

struct ImageAdvancedManipulation : public Base {

};
#pragma endregion ImageAction

}