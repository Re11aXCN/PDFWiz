#pragma once
#include <abi/abi_DocMetadata.h>
#include <string_view>

inline std::size_t GetWordPages(const std::string& file) {
	return GetWordPages(file.c_str(), file.size());
}
inline std::size_t GetExcelSheets(const std::string& file) {
	return GetExcelSheets(file.c_str(), file.size());
}
inline std::size_t GetPPTSlides(const std::string& file) {
	return GetPPTSlides(file.c_str(), file.size());
}
inline std::size_t GetPDFPages(const std::string& file, const std::string& password) {
	return GetPDFPages(file.c_str(), file.size(), password.c_str(), password.size());
}

inline std::size_t GetWordPages(std::string_view file) {
	return GetWordPages(file.data(), file.size());
}
inline std::size_t GetExcelSheets(std::string_view file) {
	return GetExcelSheets(file.data(), file.size());
}
inline std::size_t GetPPTSlides(std::string_view file) {
	return GetPPTSlides(file.data(), file.size());
}
inline std::size_t GetPDFPages(std::string_view file, std::string_view password) {
	return GetPDFPages(file.data(), file.size(), password.data(), password.size());
}

#ifdef QT_CORE_LIB
#include <QString>
#include <QStringView>
inline std::size_t GetWordPages(const QString& file) {
	return GetWordPages(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetExcelSheets(const QString& file) {
	return GetExcelSheets(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetPPTSlides(const QString& file) {
	return GetPPTSlides(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetPDFPages(const QString& file, const QString& password) {
	return GetPDFPages(file.toUtf8().data(), file.toUtf8().size(), password.toUtf8().data(), password.toUtf8().size());
}

inline std::size_t GetWordPages(const QStringView& file) {
	return GetWordPages(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetExcelSheets(const QStringView& file) {
	return GetExcelSheets(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetPPTSlides(const QStringView& file) {
	return GetPPTSlides(file.toUtf8().data(), file.toUtf8().size());
}
inline std::size_t GetPDFPages(const QStringView& file, const QStringView& password) {
	return GetPDFPages(file.toUtf8().data(), file.toUtf8().size(), password.toUtf8().data(), password.toUtf8().size());
}
#endif