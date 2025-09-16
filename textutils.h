#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <QByteArray>
#include <QLocale>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QtGlobal>

/*!
 * \brief Outils utilitaires pour manipulations de texte (Qt 5/6).
 * Classe statique, thread-safe si utilisée sans état partagé.
 */
class TextUtils
{
  public:
    // Styles de fin de ligne
    enum class LineEnding
    {
        LF,
        CRLF,
        CR
    };

    // Alignements
    enum class Align
    {
        Left,
        Right,
        Center,
        Justify
    };

    // ===========================
    // Transformations de base
    // ===========================
    static QString toUpper(const QString& text, const QLocale& locale = QLocale());
    static QString toLower(const QString& text, const QLocale& locale = QLocale());
    static QString toggleCase(const QString& text);
    static QString capitalizeWords(const QString& text, const QLocale& locale = QLocale());
    static QString capitalizeSentences(const QString& text, const QLocale& locale = QLocale());
    static QString reverseText(const QString& text);      // inversion grapheme-safe
    static QString removeDiacritics(const QString& text); // supprime les accents (décomposition Unicode)

    // ===========================
    // Espaces & mise en forme
    // ===========================
    static QString trim(const QString& text);            // trim global
    static QString trimLines(const QString& text);       // trim ligne par ligne
    static QString squeezeSpaces(const QString& text);   // compresse espaces horizontaux (pas les retours)
    static QString normalizeSpaces(const QString& text); // trim global + squeezeSpaces
    static QString tabsToSpaces(const QString& text, int tabWidth = 4);
    static QString spacesToTabs(const QString& text, int tabWidth = 4);

    static QString wrapText(const QString& text, int width, bool breakLongWords = false, LineEnding eol = LineEnding::LF);
    static QString indent(const QString& text, int count = 4, QChar fill = QLatin1Char(' '));
    static QString unindent(const QString& text, int count = 4, QChar fill = QLatin1Char(' '));

    static QString alignText(const QString& text, Align align,
                             int width);                        // par ligne
    static QString justifyLine(const QString& line, int width); // helper public

    // ===========================
    // Nettoyage
    // ===========================
    static QString removeEmptyLines(const QString& text);
    static QString removeDuplicateLines(const QString& text, bool keepFirst = true, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    static QString removeNonPrintable(const QString& text); // supprime caractères de contrôle (sauf \t \n \r)

    static QString normalizeLineEndings(const QString& text, LineEnding eol);
    static QString ensureTrailingNewline(const QString& text, LineEnding eol = LineEnding::LF);

    // ===========================
    // Analyse & statistiques
    // ===========================
    static int wordCount(const QString& text);
    static int charCount(const QString& text); // nombre de QChars (pas graphemes)
    static int lineCount(const QString& text);

    static QMap<QString, int> wordFrequency(const QString& text, Qt::CaseSensitivity cs = Qt::CaseInsensitive);
    static QStringList topWords(const QString& text, int topN = 10, Qt::CaseSensitivity cs = Qt::CaseInsensitive);

    // ===========================
    // Recherche / Remplacement
    // ===========================
    static QString replace(const QString& text,
                           const QString& pattern,
                           const QString& replacement,
                           bool useRegex = false,
                           Qt::CaseSensitivity cs = Qt::CaseSensitive,
                           bool wholeWord = false,
                           bool multiline = true,
                           bool dotMatchesNewline = false);

    static QString regexReplace(const QString& input,
                                const QString& pattern,
                                const QString& replacement,
                                Qt::CaseSensitivity cs = Qt::CaseSensitive,
                                bool multiline = true,
                                bool dotMatchesNewline = false,
                                QString* error = nullptr);

    static QString escapeRegex(const QString& literal);

    // ===========================
    // Extracteurs
    // ===========================
    static QStringList extractEmails(const QString& text);
    static QStringList extractUrls(const QString& text);
    static QStringList extractPhones(const QString& text);

    // ===========================
    // Encodage / Hachage
    // ===========================
    static QString toBase64(const QString& text);     // UTF-8 -> Base64 (ascii)
    static QString fromBase64(const QString& base64); // Base64 -> UTF-8 (QString)

    static QByteArray hashMd5(const QString& text);  // binaire
    static QByteArray hashSha1(const QString& text); // binaire
    static QString hashMd5Hex(const QString& text);  // hex string
    static QString hashSha1Hex(const QString& text); // hex string
    // Décode des bytes vers QString selon un encodage (ex: "ISO-8859-1",
    // "UTF-8")
    static QString decode(const QByteArray& data, const QString& encoding, bool* ok = nullptr);
    // Encode un QString vers des bytes selon un encodage (ex: "Windows-1252")
    static QByteArray encode(const QString& text, const QString& encoding, bool* ok = nullptr);

    // ===========================
    // Lignes (split/join/tri)
    // ===========================

    static QStringList splitLines(const QString& text, bool keepEmpty = true);
    static QStringList splitLines(const QString& text,
                                  const QString& separator,
                                  bool keepEmpty,
                                  Qt::CaseSensitivity cs,
                                  bool trimEach,
                                  bool keepSeparator);

    static QString joinLines(const QStringList& lines, LineEnding eol = LineEnding::LF);
    static QString joinLines(QString& lines);

    static QString sortLines(const QString& text,
                             bool ascending = true,
                             bool unique = false,
                             bool numeric = false,
                             Qt::CaseSensitivity cs = Qt::CaseSensitive,
                             const QLocale& locale = QLocale());

    // ===========================
    // HTML
    // ===========================
    static QString stripHtmlTags(const QString& text);

    // ===========================
    // Helpers EOL
    // ===========================
    static QString eolString(LineEnding eol);

  private:
    TextUtils() = delete; // utilitaire statique uniquement
};

#endif // TEXTUTILS_H
