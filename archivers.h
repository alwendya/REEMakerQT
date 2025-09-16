#ifndef ARCHIVERS_H
#define ARCHIVERS_H

#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QVector>

#define NOGDI // Exclut les définitions GDI incluant DrawText
#include <windows.h>
#include <xxhash.h>
#include <zstd.h>

class FastArchiver
{
  public:
    FastArchiver() {}
    enum RetourDecompression
    {
        Hash_incorrect,
        Erreur_ouverture_archive,
        Erreur_creation_dossier_destination,
        Succes
    };
    struct typeFile
    {
        bool isDir = false;
        QString relativePath;
        QString fullPath;
        bool isReadable = false;
        bool isHidden = false;
        QString xxhashFromArchive;
        quint64 fileSize = 0;
    };
    QString compressFolderBuffered(const QString& folderPath,
                                   const QString& outputFilePath,
                                   size_t bufferSize = 1 * 1024 * 1024);
    RetourDecompression decompressArchiveBuffered(const QString& archivePath,
                                                  const QString& outputFolder);
    QString computeXXH64_FileHash(const QString& filePath,
                                  size_t bufferSize = 65536);
    QVector<typeFile> retrieveContentInformation(const QString& archivePath,
                                                 const QString& outputFolder);
    void setVerbose(bool verbose) { mVerbose = verbose; }

  private:
    bool IsFileReadOnly(const std::wstring& filePath)
    {
        DWORD attributes = GetFileAttributes(filePath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            // std::cerr << "Error: Unable to get file attributes." <<
            // std::endl;
            return false; // Handle error appropriately
        }
        return (attributes & FILE_ATTRIBUTE_READONLY) != 0;
    }
    bool mVerbose = false;
};
#endif // ARCHIVERS_H
