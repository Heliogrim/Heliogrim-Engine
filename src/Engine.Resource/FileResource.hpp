#pragma once

#include <string>
#include <Engine.Common/String.hpp>

class FileResource {
public:
    /** Default constructor */
    FileResource();

    /** Destructor */
    ~FileResource();

    enum Files : int {
        // System Files
        S_SHADER_FILE_MAIN_FRAGMENT = 0,
        S_SHADER_FILE_MAIN_VERTEX = 1
    };

    enum DataType : uint8_t {
        eAsset,
        eMaterial,
        eTexture,
        eDae,
        eFbx,
        eKtx,
        eObj,
        eFlac,
        eOgg,
        eWav
    };

    /**
     * Gets a file
     *
     * @param file_ The file.
     *
     * @return The file.
     */
    static std::string getFile(Files file_);

    /**
     * Gets file ending
     *
     * @param type_ The type.
     *
     * @return The file ending.
     */
    static std::string getFileEnding(const DataType& type_);

    /**
     * Gets roots dir
     *
     * @return The roots dir.
     */
    static std::string getRootDir();

    /**
     * Gets assets directory
     * 
     * @return the assets directory.
     */
    static std::string getAssetDir();

    /**
     * Gets audio directory
     * 
     * @return the audios directory.
     */
    static std::string getAudioDir();

    /**
     * Gets textures directory
     * 
     * @return the textures directory.
     */
    static std::string getTextureDir();

    /**
     * Gets worlds directory.
     * 
     * @return the worlds directory.
     */
    static std::string getWorldDir();

private:
    /**
     * Gets current dir
     *
     * @return The current dir.
     */
    static std::string getCurrentDir();
};
