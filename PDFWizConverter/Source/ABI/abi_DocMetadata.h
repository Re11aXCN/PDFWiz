#pragma once
#include <string>

extern "C" {
	__declspec(dllimport) std::size_t GetWordPages(const char* file, std::size_t size);
	__declspec(dllimport) std::size_t GetExcelSheets(const char* file, std::size_t size);
	__declspec(dllimport) std::size_t GetPPTSlides(const char* file, std::size_t size);
	__declspec(dllimport) std::size_t GetPDFPages(const char* file, std::size_t file_size,
		const char* password, std::size_t password_size);
}