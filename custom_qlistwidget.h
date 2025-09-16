#ifndef CUSTOM_QLISTWIDGET_H
#define CUSTOM_QLISTWIDGET_H

#include "customlistitemwidget.h"
#include <QApplication>
#include <QDir>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QListWidget>
#include <QMimeData>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>
#include <iomanip>
#include <podofo/podofo.h>
#include <random>
#include <sstream>

class DraggableListWidget : public QListWidget
{
    Q_OBJECT

  public:
    DraggableListWidget(QWidget* parent = nullptr)
      : QListWidget(parent)
    {
        // Activation de la réorganisation interne
        setDragEnabled(true);
        setAcceptDrops(true);
        setDragDropMode(QAbstractItemView::InternalMove);
        setDropIndicatorShown(true);
        // setDefaultDropAction(Qt::CopyAction);
    }

    void Accept_Image(bool valeur) { m_acceptIMG = valeur; }
    void Accept_PDF(bool valeur) { m_acceptPDF = valeur; }

  protected:
    void dragEnterEvent(QDragEnterEvent* event) override
    {
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        } else
            QListWidget::dragEnterEvent(event);
    }

    void dropEvent(QDropEvent* event) override
    {
        const QMimeData* mimeData = event->mimeData();
        if (mimeData->hasUrls()) {
            QList<QUrl> urlList = mimeData->urls();
            for (const QUrl& url : std::as_const(urlList)) {
                QFileInfo fileInfo(url.toLocalFile());
                if (fileInfo.isFile()) {
                    if (fileInfo.suffix().toLower() == "pdf" && m_acceptPDF == false)
                        continue;
                    if (fileInfo.suffix().toLower() == "jpg" || fileInfo.suffix().toLower() == "jpeg" || fileInfo.suffix().toLower() == "png" ||
                        fileInfo.suffix().toLower() == "bmp" || fileInfo.suffix().toLower() == "webp" || fileInfo.suffix().toLower() == "tiff")
                        if (m_acceptIMG == false)
                            continue;
                    QListWidgetItem* item = new QListWidgetItem(this);
                    // Crée votre widget personnalisé
                    CustomListItemWidget* itemWidget = new CustomListItemWidget(fileInfo.absoluteFilePath());
                    if (itemWidget->getExtension() == CustomListItemWidget::typeFichier::PDF) {
                        QString fichierTemp =
                          QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/REEMAKER_TEMP_" + generate_random_64bit_hex() + ".pdf";
                        // Copie local
                        auto ExtendedSource = getExtendedPath(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));
                        QFile::copy(ExtendedSource, fichierTemp);
                        unsigned int NombrePage = 0;
                        {
                            try {
                                PoDoFo::PdfMemDocument document;
                                auto nomFichierPoDoFo = PoDoFo::PdfString(fichierTemp.toUtf8().toStdString());
                                document.Load(nomFichierPoDoFo);
                                NombrePage = document.GetPages().GetCount();

                            } catch (const PoDoFo::PdfError& e) {
                                NombrePage = 0;
                            }
                        }
                        itemWidget->setStartPageMax(NombrePage);
                        itemWidget->setEndPageMax(NombrePage);
                        itemWidget->setEndPage(NombrePage);
                        QFile::remove(fichierTemp);
                    }
                    // Ajoute le widget à l'item
                    setItemWidget(item, itemWidget);
                    // Ajuste la taille de l'item pour qu'elle corresponde au widget
                    item->setSizeHint(itemWidget->sizeHint());
                }
            }
            event->acceptProposedAction();
        } else {
            QListWidget::dropEvent(event);
        }
    }

    // Override dragMoveEvent to control the drag's appearance
    void startDrag(Qt::DropActions supportedActions) override
    {
        QMimeData* data = mimeData(selectedItems());
        if (!data) {
            return;
        }
        QDrag* drag = new QDrag(this);
        drag->setMimeData(data);
        drag->setPixmap(QPixmap(0, 0));

        drag->exec(supportedActions);
    }

    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override
    {
        if (items.size() > 0)
            return QListWidget::mimeData(items);
        else
            return nullptr;
    }

  private:
    QString getExtendedPath(const QString& path)
    {
        if (path.isEmpty()) {
            return path;
        }
        QString prefix = "\\\\?\\";
        if (path.startsWith("\\\\"))
            return "\\\\?\\UNC\\" + path.mid(2);
        else
            return prefix + path;
    }

    QString generate_random_64bit_hex()
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> distrib;

        uint64_t random_value = distrib(gen);

        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << random_value;
        return QString::fromStdString(ss.str());
    }

    bool m_acceptPDF = true;
    bool m_acceptIMG = true;
};

#endif // CUSTOM_QLISTWIDGET_H
