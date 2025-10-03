#pragma once

#include <NXProperty.h>
#include <variant>
namespace WizConverter::Module {
    namespace Enums {
        enum class FileType : int8_t {
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
        enum class FileState : int8_t {
            LOADING = 0,
            SUCCESS,
            FAILED,
            CORRUPTION,
            DELETED,
            BEREADY,
            PROCESSING,
            UNKONWERROR
        };

        struct MasterModule {
            enum class Type {
                PDFToWord,
                WordToPDF,
                PDFAction,
                ImageAction
            };
        };

        struct SlaveModule {};

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