#include "archivers.h"

QString
FastArchiver::compressFolderBuffered(const QString& folderPath,
                                     const QString& outputFilePath,
                                     size_t bufferSize)
{
    QElapsedTimer timer;
    timer.restart();
    uint64_t Taille = 0;
    uint64_t nbFichier = 0;
    uint64_t nbDossier = 0;
    QFile outFile(outputFilePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Erreur : impossible d'ouvrir le fichier de sortie"
                    << outputFilePath;
        return QString();
    }

    QDataStream out(&outFile);
    QDir baseDir(folderPath);
    if (!baseDir.exists()) {
        qCritical() << "Erreur : le dossier source n'existe pas" << folderPath;
        return QString();
    }

    QDirIterator it(folderPath,
                    QDir::AllEntries | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo info(filePath);
        QString relativePath = baseDir.relativeFilePath(filePath);

        bool isDir = info.isDir();
        auto xxhash = isDir ? "" : computeXXH64_FileHash(filePath);
        out << static_cast<qint32>(isDir ? 1 : 0);
        out << relativePath;
        out << static_cast<qint32>(IsFileReadOnly(filePath.toStdWString()) ? 1
                                                                           : 0);
        out << static_cast<qint32>(info.isHidden() ? 1 : 0);
        out << xxhash;
        out << static_cast<quint64>(info.size());

        Taille += info.size();
        if (isDir)
            nbDossier++;
        else
            nbFichier++;

        if (!isDir) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                qWarning() << "Impossible d'ouvrir le fichier pour lecture:"
                           << filePath;
                continue;
            }

            while (!file.atEnd()) {
                QByteArray rawData;
                rawData.append(file.read(bufferSize));
                QByteArray compressedData;
                compressedData.resize(ZSTD_compressBound(rawData.size()));
                size_t compressedSize = ZSTD_compress(compressedData.data(),
                                                      compressedData.size(),
                                                      rawData.data(),
                                                      rawData.size(),
                                                      10);

                if (ZSTD_isError(compressedSize)) {
                    qWarning() << "Erreur de compression du chunk" << filePath
                               << ":" << ZSTD_getErrorName(compressedSize);
                    continue;
                }

                compressedData.resize(compressedSize);
                out << static_cast<quint64>(compressedSize);
                out << static_cast<quint64>(rawData.size());

                if (out.writeRawData(compressedData.constData(),
                                     compressedSize) !=
                    (qint64)compressedSize) {
                    qWarning()
                      << "Erreur d'écriture des données compressées pour "
                         "le chunk"
                      << filePath;
                }
            }
        }
    }

    outFile.close();
    if (mVerbose) {
        qDebug() << "Compression terminée avec succès.";
        qDebug() << " Source :" << Taille << " en " << outFile.size()
                 << " Ratio : " << ((outFile.size() * 100.0) / (double)Taille)
                 << "% en " << timer.elapsed() << "ms"
                 << "\n  Nombre dossier : " << nbDossier
                 << "\n  Nombre fichier : " << nbFichier;
    }
    QString HashFileQS = computeXXH64_FileHash(outputFilePath);
    QFile hashFile(outputFilePath + "." + HashFileQS + ".hash");
    if (!hashFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Erreur : impossible d'ouvrir le fichier de hash"
                    << outputFilePath + "." + HashFileQS + ".hash";
        return QString();
    }
    hashFile.close();

    return HashFileQS;
}

FastArchiver::RetourDecompression
FastArchiver::decompressArchiveBuffered(const QString& archivePath,
                                        const QString& outputFolder)
{
    QElapsedTimer timer;
    timer.restart();
    uint64_t Taille = 0;
    uint64_t nbFichier = 0;
    uint64_t nbDossier = 0;

    QFile inFile(archivePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Erreur : impossible d'ouvrir l'archive" << archivePath;
        return RetourDecompression::Erreur_ouverture_archive;
    }

    auto HashFileQS = computeXXH64_FileHash(archivePath);
    if (QFile(archivePath + "." + HashFileQS + ".hash").exists()) {
        if (mVerbose)
            qDebug() << "Hash OK";
    } else {
        if (mVerbose)
            qDebug() << "Hash différent";
    }

    QDataStream in(&inFile);
    QDir outputDir(outputFolder);
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        qCritical() << "Erreur : impossible de créer le dossier de sortie"
                    << outputFolder;
        return RetourDecompression::Erreur_creation_dossier_destination;
    }

    while (!in.atEnd()) {
        qint32 type = 0;
        QString relativePath, xxhash;
        qint32 readOnlyFlag = 0, hiddenFlag = 0;
        quint64 fileOriginalSize = 0;

        in >> type >> relativePath >> readOnlyFlag >> hiddenFlag >> xxhash >>
          fileOriginalSize;
        QString fullPath = outputDir.filePath(relativePath);
        if (mVerbose)
            qDebug() << " * Entrée > " << relativePath << "("
                     << fileOriginalSize << ") Hash :" << xxhash;

        if (type == 1) {
            ///
            /// DOSSIER
            ///
            nbDossier++;
            if (!QDir().mkpath(fullPath)) {
                qWarning() << "Erreur : impossible de créer le dossier"
                           << fullPath;
            }
        } else {
            ///
            /// FICHIER
            ///
            nbFichier++;
            quint64 compressedChunkSize = 0, originalChunkSize = 0,
                    sizeRestored = 0;

            QFile outFile(fullPath);
            if (!outFile.open(QIODevice::WriteOnly)) {
                qWarning() << "Erreur : impossible d'écrire le fichier"
                           << fullPath;
                continue;
            }
            uint64_t iLoop = 0;
            while (sizeRestored != fileOriginalSize) {
                in >> compressedChunkSize >> originalChunkSize;
                Taille += originalChunkSize;
                sizeRestored += originalChunkSize;
                if (mVerbose)
                    qDebug() << "    > Loop " << iLoop
                             << " Compressed=" << compressedChunkSize
                             << " Original=" << originalChunkSize
                             << " TotalToRestore=" << fileOriginalSize
                             << " CurrentRestore=" << sizeRestored;

                QByteArray compressedData(compressedChunkSize,
                                          Qt::Uninitialized);
                if (in.readRawData(compressedData.data(),
                                   compressedChunkSize) !=
                    (qint64)compressedChunkSize) {
                    qWarning()
                      << "Erreur de lecture des données compressées pour"
                      << relativePath;
                    continue;
                }

                QByteArray decompressedData;
                decompressedData.resize(originalChunkSize);
                size_t result = ZSTD_decompress(decompressedData.data(),
                                                originalChunkSize,
                                                compressedData.data(),
                                                compressedChunkSize);

                if (ZSTD_isError(result)) {
                    qWarning() << "Erreur de décompression pour" << relativePath
                               << ":" << ZSTD_getErrorName(result);
                    continue;
                }

                qint64 writtenBytes = outFile.write(decompressedData);
                if (writtenBytes <= 0) {
                    qWarning()
                      << "Erreur d'écriture dans le fichier" << fullPath;
                    break;
                }
                iLoop++;
            }

            outFile.close();

            /// Lecture seule
            QFile::Permissions perms = QFile::permissions(fullPath);
            if (readOnlyFlag)
                perms &= ~QFile::WriteUser;
            QFile::setPermissions(fullPath, perms);
            /// Attribut "caché" à l'aide de l'API Windows
            if (hiddenFlag)
                SetFileAttributesW(fullPath.toStdWString().c_str(),
                                   FILE_ATTRIBUTE_HIDDEN);
        }
    }

    inFile.close();
    if (mVerbose)
        qDebug() << "Décompression terminée avec succès en " << timer.elapsed()
                 << "ms"
                 << "\n  Nombre dossier : " << nbDossier
                 << "\n  Nombre fichier : " << nbFichier;
    Q_UNUSED(Taille);
    return RetourDecompression::Succes;
}

QString
FastArchiver::computeXXH64_FileHash(const QString& filePath, size_t bufferSize)
{
    QElapsedTimer timer;
    timer.restart();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Impossible d'ouvrir le fichier pour le hash:"
                   << filePath;
        return QString();
    }

    XXH64_state_t* state = XXH64_createState();
    if (state == nullptr) {
        qWarning() << "Erreur : impossible de créer l'état XXH64.";
        return QString();
    }

    if (XXH64_reset(state, 0) == XXH_ERROR) {
        qWarning() << "Erreur : échec du reset XXH64.";
        XXH64_freeState(state);
        return QString();
    }

    QByteArray buffer;
    buffer.resize(bufferSize);

    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer.data(), bufferSize);
        if (bytesRead <= 0) {
            qWarning() << "Erreur de lecture du fichier pendant le hash.";
            XXH64_freeState(state);
            return QString();
        }

        if (XXH64_update(state,
                         buffer.constData(),
                         static_cast<size_t>(bytesRead)) == XXH_ERROR) {
            qWarning() << "Erreur : échec de mise à jour du hash.";
            XXH64_freeState(state);
            return QString();
        }
    }

    XXH64_hash_t hash = XXH64_digest(state);
    XXH64_freeState(state);
    if (mVerbose)
        qDebug() << "Hash terminée avec succès en " << timer.elapsed() << "ms"
                 << "["
                 << QString::asprintf("%016llx",
                                      static_cast<unsigned long long>(hash))
                 << "]";

    return QString::asprintf("%016llx", static_cast<unsigned long long>(hash));
}

QVector<FastArchiver::typeFile>
FastArchiver::retrieveContentInformation(const QString& archivePath,
                                         const QString& outputFolder)
{
    QVector<FastArchiver::typeFile> reponse;
    QElapsedTimer timer;
    timer.restart();
    uint64_t Taille = 0;
    uint64_t nbFichier = 0;
    uint64_t nbDossier = 0;

    QFile inFile(archivePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Erreur : impossible d'ouvrir l'archive" << archivePath;
        return reponse;
    }

    auto HashFileQS = computeXXH64_FileHash(archivePath);
    if (QFile(archivePath + "." + HashFileQS + ".hash").exists()) {
        if (mVerbose)
            qDebug() << "Hash OK";
    } else {
        if (mVerbose)
            qDebug() << "Hash différent";
    }

    QDataStream in(&inFile);
    QDir outputDir(outputFolder);

    while (!in.atEnd()) {
        FastArchiver::typeFile _file;
        qint32 type = 0;
        QString relativePath, xxhash;
        qint32 readOnlyFlag = 0, hiddenFlag = 0;
        quint64 fileOriginalSize = 0;

        in >> type >> relativePath >> readOnlyFlag >> hiddenFlag >> xxhash >>
          fileOriginalSize;
        QString fullPath = outputDir.filePath(relativePath);
        _file.isDir = (bool)type;
        _file.relativePath = relativePath;
        _file.isReadable = (bool)readOnlyFlag;
        _file.isHidden = (bool)hiddenFlag;
        _file.xxhashFromArchive = xxhash;
        _file.fileSize = fileOriginalSize;
        _file.fullPath = fullPath;
        reponse.append(_file);

        if (type == 1) {
            /// DOSSIER
            nbDossier++;
        } else {
            /// FICHIER
            nbFichier++;
            quint64 compressedChunkSize = 0, originalChunkSize = 0,
                    sizeRestored = 0;

            uint64_t iLoop = 0;
            while (sizeRestored != fileOriginalSize) {
                in >> compressedChunkSize >> originalChunkSize;
                Taille += originalChunkSize;
                sizeRestored += originalChunkSize;
                in.skipRawData(compressedChunkSize);
                iLoop++;
            }
            Q_UNUSED(iLoop);
        }
    }

    inFile.close();

    Q_UNUSED(Taille);
    Q_UNUSED(nbFichier);
    Q_UNUSED(nbDossier);

    return reponse;
}
