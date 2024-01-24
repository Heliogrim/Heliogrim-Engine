#include <fstream>
#include <Engine.Common/__macro.hpp>
#include <Windows.h>
#include <string>

#include "FileResource.hpp"
#include "File.hpp"

FileResource::FileResource() = default;

FileResource::~FileResource() = default;

std::string FileResource::getFile(const Files file_) {
    std::string str = getCurrentDir() + "\\resources";

    switch (file_) {
        case FileResource::S_SHADER_FILE_MAIN_FRAGMENT: {
            str += "\\shader\\frag.spv";
            break;
        }
        case FileResource::S_SHADER_FILE_MAIN_VERTEX: {
            str += "\\shader\\vert.spv";
            break;
        }

        default: {
            throw std::exception();
        }
    }
    return str;
}

std::string FileResource::getFileEnding(const DataType& type_) {
    switch (type_) {
        case eMaterial: return std::string { "mat" };
        case eDae: return std::string { "dae" };
        case eFbx: return std::string { "fbx" };
        case eKtx: return std::string { "ktx" };
        case eObj: return std::string { "obj" };
        case eFlac: return std::string { "flac" };
        case eOgg: return std::string { "ogg" };
        case eWav: return std::string { "wav" };
        default: return std::string { "dat" };
    }
}

std::string FileResource::getRootDir() {
    return getCurrentDir() + hg::fs::File::seperator + "resources";
}

std::string FileResource::getAssetDir() {
    return getRootDir() + hg::fs::File::seperator + "assets";
}

std::string FileResource::getAudioDir() {
    return getAssetDir() + hg::fs::File::seperator + "audio";
}

std::string FileResource::getTextureDir() {
    return getAssetDir() + hg::fs::File::seperator + "texture";
}

std::string FileResource::getWorldDir() {
    return getRootDir() + hg::fs::File::seperator + "world";
}

std::string FileResource::getCurrentDir() {
    char wdir[MAX_PATH + 1];
    GetCurrentDirectoryA(sizeof(wdir), wdir);
    #ifdef _DEBUG
    std::string temp = std::string(wdir);
    if (temp.compare(temp.length() - std::string("Debug").length(), std::string("Debug").length(), "Debug") == 0) {
    } else {
        temp = temp.substr(0, temp.find_last_of("/\\"));
    }
    return temp;
    #else
	return wdir;
    #endif
}
