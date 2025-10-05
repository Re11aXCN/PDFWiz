#pragma once

#include <NXProperty.h>
#include <QSize>
#include <variant>
#include <absl/container/flat_hash_map>
namespace WizConverter::Module {
    namespace Enums {
        struct FileFormat {
            enum class Type : int8_t {
                CAD,
                CAJ,
                EPUB,
                EXCEL,
                HTML,
                IMAGE,
                MARKDOWN,
                OFD,
                PDF,
                POWERPOINT,
                TXT,
                WORD,
                __END
            };
            static Type GetFileTypeByExtension(const QString& extension) {
                static const absl::flat_hash_map<QString, Type> fileTypes{
                    { "dwg", Type::CAD }, { "dxf", Type::CAD },
                    { "caj", Type::CAJ }, { "epub", Type::EPUB },
                    { "xlsx", Type::EXCEL }, { "xls", Type::EXCEL },
                    { "csv", Type::EXCEL }, { "html", Type::HTML },
                    { "htm", Type::HTML }, { "jpg", Type::IMAGE },
                    { "jpeg", Type::IMAGE }, { "png", Type::IMAGE },
                    { "bmp", Type::IMAGE }, { "tif", Type::IMAGE },
                    { "tiff", Type::IMAGE }, { "gif", Type::IMAGE },
                    { "svg", Type::IMAGE }, { "md", Type::MARKDOWN },
                    { "ofd", Type::OFD }, { "pdf", Type::PDF },
                    { "pptx", Type::POWERPOINT }, { "ppt", Type::POWERPOINT },
                    { "txt", Type::TXT }, { "docx", Type::WORD },
                    { "doc", Type::WORD }, { "rtf", Type::WORD },
                };
                const auto it = fileTypes.find(extension.toLower());
                return it != fileTypes.end() ? (*it).second : Type::__END;
            }
        };
        struct FileState {
            enum class Type : int8_t {
                LOADING = 0,
                BEREADY,
                PROCESSING,
                SUCCESS,
                FAILED,
                CORRUPTION,
                DELETED,
                UNKONWERROR
            };
        };
        struct MasterModule {
            enum class Type {
                PDFToWord,
                WordToPDF,
                PDFAction,
                ImageAction
            };
        };

        struct SlaveModule { enum class Type {}; };

        struct PDFToWord : public SlaveModule {
            enum class Type {
                PDFToWord,
                PDFToExcel,
                PDFToPowerpoint,
                PDFToImage,
                PDFToTxt,
                PDFToCad,
                PDFToEpub,
                PDFToHtml,
                PDFToMarkdown
            };
        };
        Q_DECLARE_METATYPE(PDFToWord::Type)

        struct WordToPDF : public SlaveModule {
            enum class Type {
                WordToPDF,
                ExcelToPDF,
                PowerpointToPDF,
                ImageToPDF,
                TxtToPDF,
                CadToPDF,
                EpubToPDF,
                HtmlToPDF,
                MarkdownToPDF
            };
        };
        Q_DECLARE_METATYPE(WordToPDF::Type)

        struct PDFAction : public SlaveModule {
            enum class Type {
                PDFSplit,
                PDFMerge,
                PDFCompress,
                PDFPageRenderAsImage,
                PDFPageExtract,
                PDFPageDelete,
                PDFCrypto,
                PDFWatermark,
                DocumentTranslate
            };
        };
        Q_DECLARE_METATYPE(PDFAction::Type)

        struct ImageAction : public SlaveModule {
            enum class Type {
                PDFToImage,
                ImageToPDF,
                ImageResize,
                ImageCompress,
                ImageTransfer,
                ImageWatermark,
                ImageOCR,
                ImageRotateCropFlip,
                ImageAdvancedManipulation,
            };
        };
        Q_DECLARE_METATYPE(ImageAction::Type)

        struct ModuleType {
            MasterModule::Type MasterModuleType;
            QVariant SlaveModuleType;
        };

        template<typename T>
        struct EnumMap {
            const char* Name;
            T Value;
        };
        template<typename T>
        struct EnumTraits;

        template<>
        struct EnumTraits<MasterModule> {
            using EnumType = MasterModule::Type;
            static constexpr auto Map = std::array<EnumMap<EnumType>, 4>{
                {{"PDFToWord"  , EnumType::PDFToWord},
                 {"WordToPDF"  , EnumType::WordToPDF},
                 {"PDFAction"  , EnumType::PDFAction},
                 {"ImageAction", EnumType::ImageAction}}
            };
        };

        template<>
        struct EnumTraits<PDFToWord> {
            using EnumType = PDFToWord::Type;
            static constexpr auto Map = std::array<EnumMap<EnumType>, 9>{
                {{"PDFToWord", EnumType::PDFToWord},
                    { "PDFToExcel" , EnumType::PDFToExcel },
                    { "PDFToPowerpoint", EnumType::PDFToPowerpoint },
                    { "PDFToImage" , EnumType::PDFToImage },
                    { "PDFToTxt"   , EnumType::PDFToTxt },
                    { "PDFToCad"   , EnumType::PDFToCad },
                    { "PDFToEpub"  , EnumType::PDFToEpub },
                    { "PDFToHtml"  , EnumType::PDFToHtml },
                    { "PDFToMarkdown", EnumType::PDFToMarkdown }}
            };
        };

        template<>
        struct EnumTraits<WordToPDF> {
            using EnumType = WordToPDF::Type;
            static constexpr auto Map = std::array<EnumMap<EnumType>, 9>{
                {{"WordToPDF"  , EnumType::WordToPDF},
                 {"ExcelToPDF" , EnumType::ExcelToPDF},
                 {"PowerpointToPDF", EnumType::PowerpointToPDF},
                 {"ImageToPDF" , EnumType::ImageToPDF},
                 {"TxtToPDF"   , EnumType::TxtToPDF},
                 {"CadToPDF"   , EnumType::CadToPDF},
                 {"EpubToPDF"  , EnumType::EpubToPDF},
                 {"HtmlToPDF"  , EnumType::HtmlToPDF},
                 {"MarkdownToPDF", EnumType::MarkdownToPDF}}
            };
        };

        template<>
        struct EnumTraits<PDFAction> {
            using EnumType = PDFAction::Type;
            static constexpr auto Map = std::array<EnumMap<EnumType>, 9>{
                {{"PDFSplit"   , EnumType::PDFSplit},
                 {"PDFMerge"   , EnumType::PDFMerge},
                 {"PDFCompress", EnumType::PDFCompress},
                 {"PDFPageRenderAsImage", EnumType::PDFPageRenderAsImage},
                 {"PDFPageExtract", EnumType::PDFPageExtract},
                 {"PDFPageDelete", EnumType::PDFPageDelete},
                 {"PDFCrypto"  , EnumType::PDFCrypto},
                 {"PDFWatermark", EnumType::PDFWatermark},
                 {"DocumentTranslate", EnumType::DocumentTranslate}}
            };
        };

        template<>
        struct EnumTraits<ImageAction> {
            using EnumType = ImageAction::Type;
            static constexpr auto Map = std::array<EnumMap<EnumType>, 9>{
                {{"PDFToImage", EnumType::PDFToImage},
                    { "ImageToPDF"  , EnumType::ImageToPDF },
                    { "ImageResize" , EnumType::ImageResize },
                    { "ImageCompress", EnumType::ImageCompress },
                    { "ImageTransfer", EnumType::ImageTransfer },
                    { "ImageWatermark", EnumType::ImageWatermark },
                    { "ImageOCR"    , EnumType::ImageOCR },
                    { "ImageRotateCropFlip", EnumType::ImageRotateCropFlip },
                    { "ImageAdvancedManipulation", EnumType::ImageAdvancedManipulation }}
            };
        };

        using SlaveModuleVariant = std::variant<
            std::monostate,
            std::array<EnumMap<PDFToWord::Type>, 9>,
            std::array<EnumMap<WordToPDF::Type>, 9>,
            std::array<EnumMap<PDFAction::Type>, 9>,
            std::array<EnumMap<ImageAction::Type>, 9>
        >;

        inline SlaveModuleVariant getSlaveModuleType(MasterModule::Type type) {
            switch (type) {
            case MasterModule::Type::PDFToWord: return EnumTraits<PDFToWord>::Map;
            case MasterModule::Type::WordToPDF: return EnumTraits<WordToPDF>::Map;
            case MasterModule::Type::PDFAction: return EnumTraits<PDFAction>::Map;
            case MasterModule::Type::ImageAction: return EnumTraits<ImageAction>::Map;
            default: return std::monostate{};
            }
        }
    }
}

using FileStateType = WizConverter::Module::Enums::FileState::Type;
using FileFormatType = WizConverter::Module::Enums::FileFormat::Type;

#define TABLE_VIEW_CHECKICON_ADJUST 16, 5, 11, -1
constexpr QSize TABLE_VIEW_CHECKICON_SIZE{ 24, 24 };
