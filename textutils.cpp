#include "textutils.h"

#include "TextUtils.h"

#include <QCollator>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextBoundaryFinder>
#include <QVector>
#include <algorithm>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#include <QStringEncoder>
#else
#include <QTextCodec>
#endif

// ===========================
// Helpers internes
// ===========================

static inline bool
isSentenceTerminator(QChar c)
{
    return c == QLatin1Char('.') || c == QLatin1Char('!') || c == QLatin1Char('?');
}

QString
TextUtils::eolString(TextUtils::LineEnding eol)
{
    switch (eol) {
        case LineEnding::LF:
            return QStringLiteral("\n");
        case LineEnding::CRLF:
            return QStringLiteral("\r\n");
        case LineEnding::CR:
            return QStringLiteral("\r");
    }
    return QStringLiteral("\n");
}

// ===========================
// Transformations
// ===========================

QString
TextUtils::toUpper(const QString& text, const QLocale& locale)
{
    return locale.toUpper(text);
}

QString
TextUtils::toLower(const QString& text, const QLocale& locale)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    return locale.toLower(text);
#else
    return text.toLower();
#endif
}

QString
TextUtils::toggleCase(const QString& text)
{
    QString out;
    out.reserve(text.size());
    for (QChar c : text) {
        if (c.isUpper())
            out.append(c.toLower());
        else if (c.isLower())
            out.append(c.toUpper());
        else
            out.append(c);
    }
    return out;
}

QString
TextUtils::capitalizeWords(const QString& text, const QLocale& locale)
{
    // Utilise un découpage par mots Unicode
    QString out = text;
    QRegularExpression re(QStringLiteral("\\p{L}[\\p{L}\\p{Mn}\\p{Nd}_]*"));
    QRegularExpressionMatchIterator it = re.globalMatch(out);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        const int start = m.capturedStart();
        const int len = m.capturedLength();
        if (len <= 0)
            continue;
        QString word = out.mid(start, len);
        if (!word.isEmpty()) {
            QString first = word.left(1);
            QString rest = word.mid(1);
            first = toUpper(first, locale);
            // Par défaut, on laisse le reste tel quel. Si tu préfères Title
            // Case strict : rest = toLower(rest, locale);
            out.replace(start, len, first + rest);
        }
    }
    return out;
}

QString
TextUtils::capitalizeSentences(const QString& text, const QLocale& locale)
{
    QString out;
    out.reserve(text.size());
    bool capitalizeNext = true;
    bool seenTerminator = false;

    for (int i = 0; i < text.size(); ++i) {
        QChar c = text.at(i);

        if (capitalizeNext && c.isLetter()) {
            QString s(c);
            s = toUpper(s, locale);
            out.append(s);
            capitalizeNext = false;
        } else {
            out.append(c);
        }

        if (isSentenceTerminator(c)) {
            seenTerminator = true;
        } else if (c.isSpace()) {
            if (seenTerminator) {
                capitalizeNext = true;
                seenTerminator = false;
            }
        } else {
            seenTerminator = false;
        }
    }
    return out;
}

QString
TextUtils::reverseText(const QString& text)
{

    QTextBoundaryFinder f(QTextBoundaryFinder::Grapheme, text);
    f.toStart();

    auto prev = f.position();
    QVector<QString> clusters;
    while (true) {
        qsizetype next = f.toNextBoundary();
        if (next < 0)
            break;
        clusters.append(text.mid(prev, next - prev));
        prev = next;
    }

    // Inverser les graphèmes
    std::reverse(clusters.begin(), clusters.end());

    // Recomposer la chaîne
    QString out;
    out.reserve(text.size());
    for (const QString& g : clusters)
        out += g;
    return out;
}

QString
TextUtils::removeDiacritics(const QString& text)
{
    // Décomposition canonique + suppression des marques d’accent (\p{Mn}) +
    // recomposition
    QString decomposed = text.normalized(QString::NormalizationForm_D);
    static const QRegularExpression reMn(QStringLiteral("\\p{Mn}+")); // marks non-spacing
    QString stripped = decomposed;
    stripped.replace(reMn, QString());
    return stripped.normalized(QString::NormalizationForm_C);
}

// ===========================
// Espaces & mise en forme
// ===========================

QString
TextUtils::trim(const QString& text)
{
    return text.trimmed();
}

QString
TextUtils::trimLines(const QString& text)
{
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    for (const QString& l : lines)
        out << l.trimmed();
    return joinLines(out, LineEnding::LF);
}

QString
TextUtils::squeezeSpaces(const QString& text)
{

    // \h = horizontal whitespace (Unicode), ne matche pas \n/\r
    static const QRegularExpression re(QStringLiteral("\\h+"), QRegularExpression::UseUnicodePropertiesOption);
    QString out = text;
    out.replace(re, QStringLiteral(" "));
    return out;
}

QString
TextUtils::normalizeSpaces(const QString& text)
{
    return squeezeSpaces(trim(text));
}

QString
TextUtils::tabsToSpaces(const QString& text, int tabWidth)
{
    if (tabWidth <= 0)
        return text;
    QString spaces(tabWidth, QLatin1Char(' '));
    QString out = text;
    out.replace(QLatin1Char('\t'), spaces);
    return out;
}

QString
TextUtils::spacesToTabs(const QString& text, int tabWidth)
{
    if (tabWidth <= 0)
        return text;
    QString out = text;
    const QRegularExpression re(QString(tabWidth, QLatin1Char(' ')));
    out.replace(re, QStringLiteral("\t"));
    return out;
}

QString
TextUtils::wrapText(const QString& text, int width, bool breakLongWords, LineEnding eol)
{
    if (width <= 0)
        return text;
    const QStringList lines = splitLines(text, true);
    QStringList wrapped;
    wrapped.reserve(lines.size());

    for (const QString& line : lines) {
        if (line.length() <= width) {
            wrapped << line;
            continue;
        }

        // Greedy wrap sur séquences non blanches
        QString current;
        QRegularExpression reWord(QStringLiteral("\\S+"));
        QRegularExpressionMatchIterator it = reWord.globalMatch(line);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            QString token = m.captured();

            if (token.length() > width && breakLongWords) {
                // Flush la ligne courante
                if (!current.isEmpty()) {
                    wrapped << current.trimmed();
                    current.clear();
                }
                for (int i = 0; i < token.length(); i += width) {
                    wrapped << token.mid(i, width);
                }
                continue;
            }

            if (current.isEmpty()) {
                current = token;
            } else if (current.length() + 1 + token.length() <= width) {
                current += QLatin1Char(' ');
                current += token;
            } else {
                wrapped << current;
                current = token;
            }
        }
        if (!current.isEmpty())
            wrapped << current;
        if (line.isEmpty())
            wrapped << QString(); // préserver ligne vide
    }

    return wrapped.join(eolString(eol));
}

QString
TextUtils::indent(const QString& text, int count, QChar fill)
{
    if (count <= 0)
        return text;
    QString prefix(count, fill);
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    for (const QString& l : lines)
        out << (prefix + l);
    return joinLines(out, LineEnding::LF);
}

QString
TextUtils::unindent(const QString& text, int count, QChar fill)
{
    if (count <= 0)
        return text;
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    for (const QString& l : lines) {
        int removeCount = 0;
        while (removeCount < count && removeCount < l.size() && l.at(removeCount) == fill) {
            ++removeCount;
        }
        out << l.mid(removeCount);
    }
    return joinLines(out, LineEnding::LF);
}

QString
TextUtils::justifyLine(const QString& line, int width)
{
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || trimmed.length() >= width)
        return trimmed;

    // Split tokens
    const QStringList words = trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (words.size() <= 1)
        return trimmed; // pas de justification possible

    int chars = 0;
    for (const QString& w : words)
        chars += w.length();
    int spacesNeeded = width - chars;
    int gaps = words.size() - 1;
    int base = spacesNeeded / gaps;
    int extra = spacesNeeded % gaps;

    QString out;
    out.reserve(width);
    for (int i = 0; i < words.size(); ++i) {
        out += words.at(i);
        if (i < gaps) {
            out += QString(base + (i < extra ? 1 : 0), QLatin1Char(' '));
        }
    }
    return out;
}

QString
TextUtils::alignText(const QString& text, Align align, int width)
{
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    for (const QString& l : lines) {
        const QString s = l;
        int len = s.length();
        if (len >= width || align == Align::Left) {
            out << s;
            continue;
        }
        int pad = width - len;
        switch (align) {
            case Align::Left:
                out << s;
                break;
            case Align::Right:
                out << (QString(pad, QLatin1Char(' ')) + s);
                break;
            case Align::Center: {
                int left = pad / 2;
                int right = pad - left;
                out << (QString(left, QLatin1Char(' ')) + s + QString(right, QLatin1Char(' ')));
                break;
            }
            case Align::Justify:
                out << justifyLine(s, width);
                break;
        }
    }
    return joinLines(out, LineEnding::LF);
}

// ===========================
// Nettoyage
// ===========================

QString
TextUtils::removeEmptyLines(const QString& text)
{
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    for (const QString& l : lines)
        if (!l.trimmed().isEmpty())
            out << l;
    return joinLines(out, LineEnding::LF);
}

QString
TextUtils::removeDuplicateLines(const QString& text, bool keepFirst, Qt::CaseSensitivity cs)
{
    const QStringList lines = splitLines(text, true);
    QStringList out;
    out.reserve(lines.size());
    QSet<QString> seen;
    for (const QString& l : lines) {
        QString key = (cs == Qt::CaseSensitive) ? l : l.toLower();
        bool exists = seen.contains(key);
        if (!exists || (exists && !keepFirst)) {
            if (!exists)
                seen.insert(key);
            out << l;
        }
    }
    if (!keepFirst) {
        // Variante: si keepFirst=false, on ne conserve que la dernière
        // occurrence : On reconstruit en ne prenant que la dernière. Simple
        // implémentation alternative :
        QSet<QString> emitted;
        QStringList reversed = out;
        std::reverse(reversed.begin(), reversed.end());
        QStringList deduped;
        for (const QString& l : reversed) {
            QString key = (cs == Qt::CaseSensitive) ? l : l.toLower();
            if (!emitted.contains(key)) {
                emitted.insert(key);
                deduped << l;
            }
        }
        std::reverse(deduped.begin(), deduped.end());
        return joinLines(deduped, LineEnding::LF);
    }
    return joinLines(out, LineEnding::LF);
}

QString
TextUtils::removeNonPrintable(const QString& text)
{
    // Supprime C0 et DEL (0x00-0x1F,0x7F) sauf \t,\n,\r
    static const QRegularExpression re(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]"));
    QString out = text;
    out.replace(re, QString());
    return out;
}

QString
TextUtils::normalizeLineEndings(const QString& text, LineEnding eol)
{
    QString tmp = text;
    tmp.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    tmp.replace(QChar::CarriageReturn, QChar::LineFeed); // \r -> \n
    const QString target = eolString(eol);
    if (target == QStringLiteral("\n"))
        return tmp;
    return tmp.replace(QStringLiteral("\n"), target);
}

QString
TextUtils::ensureTrailingNewline(const QString& text, LineEnding eol)
{
    const QString e = eolString(eol);
    if (text.endsWith(e))
        return text;
    // Harmoniser les EOL au passage
    QString tmp = normalizeLineEndings(text, eol);
    if (!tmp.endsWith(e))
        tmp += e;
    return tmp;
}

// ===========================
// Analyse & statistiques
// ===========================

int
TextUtils::wordCount(const QString& text)
{
    int count = 0;
    QRegularExpression re(QStringLiteral("\\p{L}[\\p{L}\\p{Mn}\\p{Nd}_]*"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        it.next();
        ++count;
    }
    return count;
}

int
TextUtils::charCount(const QString& text)
{
    return text.size();
}

int
TextUtils::lineCount(const QString& text)
{
    if (text.isEmpty())
        return 0;
    // Normaliser pour compter correctement
    QString tmp = text;
    tmp.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    tmp.replace(QChar::CarriageReturn, QChar::LineFeed);
    return tmp.count(QLatin1Char('\n')) + 1;
}

QMap<QString, int>
TextUtils::wordFrequency(const QString& text, Qt::CaseSensitivity cs)
{
    QMap<QString, int> freq;
    QRegularExpression re(QStringLiteral("\\p{L}[\\p{L}\\p{Mn}\\p{Nd}_]*"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        QString w = m.captured();
        if (cs == Qt::CaseInsensitive)
            w = w.toLower();
        freq[w] += 1;
    }
    return freq;
}

QStringList
TextUtils::topWords(const QString& text, int topN, Qt::CaseSensitivity cs)
{

    QMap<QString, int> freq = wordFrequency(text, cs);

    QVector<QPair<QString, int>> v;
    v.reserve(freq.size());
    for (auto it = freq.cbegin(); it != freq.cend(); ++it) {
        // qMakePair évite les soucis d'initialisation avec {}
        v.append(qMakePair(it.key(), it.value()));
    }

    std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second)
            return a.second > b.second; // tri décroissant par fréquence
        return a.first < b.first;       // à fréquence égale: tri alpha ascendant
    });

    QStringList out;
    out.reserve(qMin(topN, v.size()));
    for (int i = 0; i < v.size() && i < topN; ++i)
        out << v.at(i).first;

    return out;
}

// ===========================
// Recherche / Remplacement
// ===========================

QString
TextUtils::replace(const QString& text,
                   const QString& pattern,
                   const QString& replacement,
                   bool useRegex,
                   Qt::CaseSensitivity cs,
                   bool wholeWord,
                   bool multiline,
                   bool dotMatchesNewline)
{
    if (!useRegex) {
        if (!wholeWord && cs == Qt::CaseSensitive) {
            return QString(text).replace(pattern, replacement, Qt::CaseSensitive);
        }
        // Pour gérer wholeWord/case-insensitive avec sécurité, on passe par une
        // RegExp échappée
        QString pat = QRegularExpression::escape(pattern);
        useRegex = true;
        // on laisse la suite gérer avec options RegExp
        return replace(text, pat, replacement, true, cs, wholeWord, multiline, dotMatchesNewline);
    }

    QString pat = pattern;
    if (wholeWord) {
        // Ajoute \b (limites de mot) si pertinent
        pat = QStringLiteral("\\b") + pat + QStringLiteral("\\b");
    }

    QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
    if (cs == Qt::CaseInsensitive)
        opts |= QRegularExpression::CaseInsensitiveOption;
    if (multiline)
        opts |= QRegularExpression::MultilineOption;
    if (dotMatchesNewline)
        opts |= QRegularExpression::DotMatchesEverythingOption;

    QRegularExpression re(pat, opts);
    QString out = text;
    out.replace(re, replacement);
    return out;
}

QString
TextUtils::regexReplace(const QString& input,
                        const QString& pattern,
                        const QString& replacement,
                        Qt::CaseSensitivity cs,
                        bool multiline,
                        bool dotMatchesNewline,
                        QString* error)
{
    QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
    if (cs == Qt::CaseInsensitive)
        opts |= QRegularExpression::CaseInsensitiveOption;
    if (multiline)
        opts |= QRegularExpression::MultilineOption;
    if (dotMatchesNewline)
        opts |= QRegularExpression::DotMatchesEverythingOption;

    QRegularExpression re(pattern, opts);
    if (!re.isValid()) {
        if (error) {
            *error = QStringLiteral("Motif REGEX invalide (%1) à l’offset %2").arg(re.errorString()).arg(re.patternErrorOffset());
        }
        return input; // inchangé si le motif est invalide
    }

    // Remplacement global (toutes les occurrences)
    QString out = input;
    out.replace(re, replacement);
    return out;
}

QString
TextUtils::escapeRegex(const QString& literal)
{
    return QRegularExpression::escape(literal);
}

// ===========================
// Extracteurs
// ===========================

QStringList
TextUtils::extractEmails(const QString& text)
{
    // Pattern simplifié, case-insensitive
    QRegularExpression re(QStringLiteral(R"((?:[A-Z0-9._%+\-]+)@(?:[A-Z0-9.\-]+)\.(?:[A-Z]{2,}))"), QRegularExpression::CaseInsensitiveOption);
    QStringList out;
    auto it = re.globalMatch(text);
    while (it.hasNext())
        out << it.next().captured();
    out.removeDuplicates();
    return out;
}

QStringList
TextUtils::extractUrls(const QString& text)
{
    // Simple URL (http/https/ftp) ; robustesse suffisante pour des utilitaires
    QRegularExpression re(QStringLiteral(R"(((?:https?|ftp)://[^\s<>"']+))"), QRegularExpression::CaseInsensitiveOption);
    QStringList out;
    auto it = re.globalMatch(text);
    while (it.hasNext())
        out << it.next().captured(1);
    out.removeDuplicates();
    return out;
}

QStringList
TextUtils::extractPhones(const QString& text)
{
    // Heuristique large (préfixe +, espaces, (), -, .)
    QRegularExpression re(QStringLiteral(R"((\+?\d[\d\s().\-]{6,}\d))"));
    QStringList out;
    auto it = re.globalMatch(text);
    while (it.hasNext())
        out << it.next().captured(1);
    out.removeDuplicates();
    return out;
}

// ===========================
// Encodage / Hachage
// ===========================

QString
TextUtils::toBase64(const QString& text)
{
    return QString::fromLatin1(text.toUtf8().toBase64());
}

QString
TextUtils::fromBase64(const QString& base64)
{
    QByteArray bytes = QByteArray::fromBase64(base64.toLatin1());
    return QString::fromUtf8(bytes);
}

QByteArray
TextUtils::hashMd5(const QString& text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5);
}

QByteArray
TextUtils::hashSha1(const QString& text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha1);
}

QString
TextUtils::hashMd5Hex(const QString& text)
{
    return QString::fromLatin1(hashMd5(text).toHex());
}

QString
TextUtils::hashSha1Hex(const QString& text)
{
    return QString::fromLatin1(hashSha1(text).toHex());
}

QString
TextUtils::decode(const QByteArray& data, const QString& encoding, bool* ok)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto enc = QStringDecoder::encodingForName(encoding.toUtf8().constData());
    if (!enc.has_value()) {
        if (ok)
            *ok = false;
        return QString();
    }
    QStringDecoder decoder(enc.value());
    QString out = decoder.decode(data);
    if (ok)
        *ok = true;
    return out;
#else
    QTextCodec* codec = QTextCodec::codecForName(encoding.toUtf8());
    if (!codec) {
        if (ok)
            *ok = false;
        return QString();
    }
    if (ok)
        *ok = true;
    return codec->toUnicode(data);
#endif
}

QByteArray
TextUtils::encode(const QString& text, const QString& encoding, bool* ok)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto enc = QStringEncoder::encodingForName(encoding.toUtf8().constData());
    if (!enc.has_value()) {
        if (ok)
            *ok = false;
        return QByteArray();
    }
    QStringEncoder encoder(enc.value());
    QByteArray out = encoder.encode(text);
    if (ok)
        *ok = true;
    return out;
#else
    QTextCodec* codec = QTextCodec::codecForName(encoding.toUtf8());
    if (!codec) {
        if (ok)
            *ok = false;
        return QByteArray();
    }
    if (ok)
        *ok = true;
    return codec->fromUnicode(text);
#endif
}

// ===========================
// Lignes (split/join/tri)
// ===========================

QStringList
TextUtils::splitLines(const QString& text, bool keepEmpty)
{
    return text.split(QRegularExpression(QStringLiteral("\\r\\n|\\r|\\n")), keepEmpty ? Qt::KeepEmptyParts : Qt::SkipEmptyParts);
}

QStringList
TextUtils::splitLines(const QString& text, const QString& separator, bool keepEmpty, Qt::CaseSensitivity cs, bool trimEach, bool keepSeparator)
{
    QStringList result;
    if (text.isEmpty())
        return result;
    if (separator.isEmpty()) {
        result.append(text);
        if (trimEach) {
            result[0] = result[0].trimmed();
        }
        return result;
    }
    bool isLineSeparator = (separator == "\n" || separator == "\r\n" || separator == "\r");
    int startPos = 0;
    int foundPos = 0;
    while ((foundPos = text.indexOf(separator, startPos, cs)) != -1) {
        QString part = text.mid(startPos, foundPos - startPos);
        if (!part.isEmpty() || keepEmpty) {
            result.append(part);
        }

        startPos = foundPos + separator.length();
    }
    if (startPos <= text.length()) {
        QString lastPart = text.mid(startPos);
        if (!lastPart.isEmpty() || keepEmpty) {
            result.append(lastPart);
        }
    }
    if (keepSeparator && !isLineSeparator) {
        for (int i = 1; i < result.size(); ++i) {
            result[i] = separator + result[i];
        }
    }

    QStringList CuratedResult;
    for (auto& var : result) {
        auto _var = splitLines(var, keepEmpty);
        for (auto lVar : _var)
            CuratedResult.append(trimEach ? lVar.trimmed() : lVar);
    }
    return CuratedResult;
}

QString
TextUtils::joinLines(const QStringList& lines, LineEnding eol)
{
    return lines.join(eolString(eol));
}

QString
TextUtils::joinLines(QString& lines)
{
    return lines.replace("\r\n", "").replace("\r", "").replace("\n", "");
}

QString
TextUtils::sortLines(const QString& text, bool ascending, bool unique, bool numeric, Qt::CaseSensitivity cs, const QLocale& locale)
{
    QStringList lines = splitLines(text, true);

    // Tri via QCollator (respecte locale, option numeric natural sort)
    QCollator collator(locale);
    collator.setCaseSensitivity(cs);
    collator.setNumericMode(numeric);

    std::stable_sort(lines.begin(), lines.end(), [&collator, ascending](const auto& a, const auto& b) {
        int cmp = collator.compare(a, b);
        return ascending ? (cmp < 0) : (cmp > 0);
    });

    if (unique) {
        QStringList deduped;
        QString last;
        for (const QString& l : lines) {
            if (deduped.isEmpty() || collator.compare(l, last) != 0) {
                deduped << l;
                last = l;
            }
        }
        lines = deduped;
    }

    return joinLines(lines, LineEnding::LF);
}

// ===========================
// HTML
// ===========================

QString
TextUtils::stripHtmlTags(const QString& text)
{
    // Suppression simple des balises. Pour un rendu avancé, envisager QTextDocumentFragment::fromHtml (QtGui).
    static const QRegularExpression tags(QStringLiteral("<[^>]+>"));
    QString out = text;
    out.replace(tags, QString());
    return out;
}
