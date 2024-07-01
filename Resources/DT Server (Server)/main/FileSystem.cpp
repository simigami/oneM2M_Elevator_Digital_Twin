#include "FileSystem.h"
#include <Windows.h>
#include <shellapi.h>

bool FileSystem::checkFileCriteria(const std::wstring file_path)
{
    // Check file length
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;
    if (!GetFileAttributesEx(file_path.c_str(), GetFileExInfoStandard, &fileAttr)) {
        std::cerr << "Error getting file attributes.\n";
        return false;
    }
    if (file_path.size() > 1024) {
        std::cerr << "File Name exceeds 1024 bytes." << file_path.size() <<  std::endl;
        return false;
    }

    // Check file extension
    std::wstring::size_type dotIndex = file_path.find_last_of(L".");
    if (dotIndex == std::wstring::npos || (file_path.substr(dotIndex) != L".txt" && file_path.substr(dotIndex) != L".csv") ) {
        std::cerr << "File extension is not '.txt'.\n";
        return false;
    }

    return true;
}

std::string FileSystem::chooseLog()
{
    OPENFILENAME ofn;       // structure to store information about the file dialog
    TCHAR szFile[512];      // buffer to store the selected file name

    // Get parent directory of current directory
    std::wstring log_dir_path = L"";
    std::wstring log_path = L"";

    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("All Files\0*.*\0");   // Filter for file types
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = log_dir_path.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    do {
        if (GetOpenFileName(&ofn) == TRUE) {
            // User selected a file, you can use ofn.lpstrFile to get the selected file path.
            log_path = ofn.lpstrFile;

            if (checkFileCriteria(log_path))
            {
                MessageBox(NULL, ofn.lpstrFile, TEXT("Selected file is valid"), MB_OK);
                break;
            }
            else
            {
                MessageBox(NULL, TEXT("Please select a valid file"), TEXT("Invalid File"), MB_OK | MB_ICONERROR);
                log_path = L"";
            }
        }

        else
        {
            // User cancelled the dialog
            MessageBox(NULL, TEXT("No file was selected."), TEXT("No File Selected"), MB_OK | MB_ICONERROR);
            break;
        }
    } while (true);

    if (log_path == L"")
    {
        std::cerr << "No file was selected.\n";
        exit(1);
    }

    // Convert log_path wstring to string
    std::string log_path_str(log_path.begin(), log_path.end());

    return log_path_str;
}

FileSystem::FileSystem()
{

}
