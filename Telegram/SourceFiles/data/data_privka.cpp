/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "data/data_privka.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "lang/lang_keys.h"

#include <QtCore/QRegularExpression>

namespace Data {
namespace {

constexpr auto kMaxEntities = 24;
constexpr auto kMinPhoneDigits = 7;
constexpr auto kMaxPhoneDigits = 15;

[[nodiscard]] const char *FeatureKey(PrivkaFeature feature) {
	switch (feature) {
	case PrivkaFeature::AntiDelete: return "privka-anti-delete";
	case PrivkaFeature::ManualRead: return "privka-manual-read";
	case PrivkaFeature::KeepTtlMessages: return "privka-keep-ttl-messages";
	case PrivkaFeature::NoStoriesRead: return "privka-no-stories-read";
	case PrivkaFeature::SaveStories: return "privka-save-stories";
	case PrivkaFeature::CopyOnDoubleClick: return "privka-copy-on-double-click";
	}
	return "";
}

[[nodiscard]] const char *EntityKey(PrivkaEntityType type) {
	switch (type) {
	case PrivkaEntityType::Phone: return "privka-extract-phone";
	case PrivkaEntityType::Username: return "privka-extract-username";
	case PrivkaEntityType::Collectible: return "privka-extract-collectible";
	case PrivkaEntityType::Crypto: return "privka-extract-crypto";
	case PrivkaEntityType::Hashtag: return "privka-extract-hashtag";
	case PrivkaEntityType::Url: return "privka-extract-url";
	}
	return "";
}

[[nodiscard]] const QRegularExpression &UrlPattern() {
	static const auto result = QRegularExpression(
		u"(?:https?://|tg://|www\\.|t\\.me/)[^\\s<>\"']+"_q);
	return result;
}

[[nodiscard]] const QRegularExpression &UsernamePattern() {
	static const auto result = QRegularExpression(
		u"@[A-Za-z][A-Za-z0-9_]{4,31}"_q);
	return result;
}

[[nodiscard]] const QRegularExpression &HashtagPattern() {
	static const auto result = QRegularExpression(
		u"#[^\\s#@,.:;!?()\\[\\]{}\"'\\\\]{1,64}"_q);
	return result;
}

[[nodiscard]] const QRegularExpression &CryptoPattern() {
	static const auto result = QRegularExpression(
		u"(?<![0-9A-Za-z_-])(?:bc1[ac-hj-np-z02-9]{11,71}"
		u"|ltc1[ac-hj-np-z02-9]{11,71}"
		u"|[13][a-km-zA-HJ-NP-Z1-9]{25,34}"
		u"|[LM3][a-km-zA-HJ-NP-Z1-9]{26,33}"
		u"|0x[a-fA-F0-9]{40}"
		u"|T[1-9A-HJ-NP-Za-km-z]{33}"
		u"|U[Qq][0-9A-Za-z_-]{46})(?![0-9A-Za-z_-])"_q);
	return result;
}

[[nodiscard]] const QRegularExpression &PhonePlusPattern() {
	static const auto result = QRegularExpression(
		u"[+][0-9](?:[0-9\\s\\-().]{5,})[0-9]"_q);
	return result;
}

[[nodiscard]] const QRegularExpression &PhoneGroupedPattern() {
	static const auto result = QRegularExpression(
		u"[0-9]{3}[\\s\\-][0-9]{3}[\\s\\-][0-9]{2}[\\s\\-][0-9]{2}"_q);
	return result;
}

[[nodiscard]] bool IsTrailingPunctuation(QChar ch) {
	static const auto punctuation = u".,;:!?)]}'\""_q;
	return punctuation.contains(ch);
}

[[nodiscard]] int DigitsCount(const QString &value) {
	auto result = 0;
	for (const auto &ch : value) {
		if (ch.isDigit()) {
			++result;
		}
	}
	return result;
}

[[nodiscard]] bool IsPhone(const QString &value) {
	const auto digits = DigitsCount(value);
	return (digits >= kMinPhoneDigits) && (digits <= kMaxPhoneDigits);
}

[[nodiscard]] bool IsHashtag(const QString &value) {
	return value.size() > 1 && value.at(1).isLetter();
}

[[nodiscard]] bool IsCollectibleUrl(const QString &value) {
	return value.contains(u"collectible"_q, Qt::CaseInsensitive)
		|| value.contains(u"/nft/"_q, Qt::CaseInsensitive)
		|| value.contains(u"fragment.com/"_q, Qt::CaseInsensitive);
}

} // namespace

const std::vector<PrivkaFeature> &PrivkaFeatures() {
	static const auto result = std::vector<PrivkaFeature>{
		PrivkaFeature::AntiDelete,
		PrivkaFeature::ManualRead,
		PrivkaFeature::KeepTtlMessages,
		PrivkaFeature::NoStoriesRead,
		PrivkaFeature::SaveStories,
		PrivkaFeature::CopyOnDoubleClick,
	};
	return result;
}

bool PrivkaEnabled(PrivkaFeature feature) {
	return Core::App().settings().readPref<bool>(
		FeatureKey(feature),
		true);
}

void SetPrivkaEnabled(PrivkaFeature feature, bool enabled) {
	Core::App().settings().writePref<bool>(FeatureKey(feature), enabled);
}

QString PrivkaFeatureTitle(PrivkaFeature feature) {
	switch (feature) {
	case PrivkaFeature::AntiDelete:
		return tr::lng_privka_anti_delete(tr::now);
	case PrivkaFeature::ManualRead:
		return tr::lng_privka_manual_read(tr::now);
	case PrivkaFeature::KeepTtlMessages:
		return tr::lng_privka_keep_ttl_messages(tr::now);
	case PrivkaFeature::NoStoriesRead:
		return tr::lng_privka_no_stories_read(tr::now);
	case PrivkaFeature::SaveStories:
		return tr::lng_privka_save_stories(tr::now);
	case PrivkaFeature::CopyOnDoubleClick:
		return tr::lng_privka_copy_on_double_click(tr::now);
	}
	return QString();
}

const std::vector<PrivkaEntityType> &PrivkaEntityTypes() {
	static const auto result = std::vector<PrivkaEntityType>{
		PrivkaEntityType::Phone,
		PrivkaEntityType::Username,
		PrivkaEntityType::Collectible,
		PrivkaEntityType::Crypto,
		PrivkaEntityType::Hashtag,
		PrivkaEntityType::Url,
	};
	return result;
}

bool PrivkaEntityEnabled(PrivkaEntityType type) {
	return Core::App().settings().readPref<bool>(
		EntityKey(type),
		true);
}

void SetPrivkaEntityEnabled(PrivkaEntityType type, bool enabled) {
	Core::App().settings().writePref<bool>(EntityKey(type), enabled);
}

QString PrivkaEntityTypeTitle(PrivkaEntityType type) {
	switch (type) {
	case PrivkaEntityType::Phone:
		return tr::lng_privka_extract_phone(tr::now);
	case PrivkaEntityType::Username:
		return tr::lng_privka_extract_username(tr::now);
	case PrivkaEntityType::Collectible:
		return tr::lng_privka_extract_collectible(tr::now);
	case PrivkaEntityType::Crypto:
		return tr::lng_privka_extract_crypto(tr::now);
	case PrivkaEntityType::Hashtag:
		return tr::lng_privka_extract_hashtag(tr::now);
	case PrivkaEntityType::Url:
		return tr::lng_privka_extract_url(tr::now);
	}
	return QString();
}

std::vector<PrivkaEntity> PrivkaEntities(const QString &text) {
	auto result = std::vector<PrivkaEntity>();
	if (text.isEmpty()) {
		return result;
	}
	auto urlSpans = std::vector<std::pair<int, int>>();
	const auto collect = [&](
			PrivkaEntityType type,
			const QRegularExpression &expression,
			auto filter) {
		if (!PrivkaEntityEnabled(type)) {
			return;
		}
		auto matches = expression.globalMatch(text);
		while (matches.hasNext()) {
			const auto match = matches.next();
			const auto position = int(match.capturedStart());
			const auto length = int(match.capturedLength());
			const auto inUrl = ranges::any_of(urlSpans, [&](const auto &span) {
				return (position < span.second)
					&& (span.first < position + length);
			});
			if (inUrl) {
				continue;
			}
			auto value = match.captured().trimmed();
			while (!value.isEmpty() && IsTrailingPunctuation(value.back())) {
				value.chop(1);
			}
			if (value.isEmpty() || !filter(value)) {
				continue;
			}
			const auto realType = (type == PrivkaEntityType::Url)
				&& IsCollectibleUrl(value)
				? PrivkaEntityType::Collectible
				: type;
			if ((realType != type) && !PrivkaEntityEnabled(realType)) {
				continue;
			}
			if (ranges::contains(result, value, &PrivkaEntity::value)) {
				continue;
			}
			result.push_back({ .type = realType, .value = value });
			if (type == PrivkaEntityType::Url) {
				urlSpans.push_back({ position, position + length });
			}
			if (int(result.size()) >= kMaxEntities) {
				return;
			}
		}
	};
	const auto any = [](const QString &) { return true; };

	collect(PrivkaEntityType::Url, UrlPattern(), any);
	collect(PrivkaEntityType::Username, UsernamePattern(), any);
	collect(PrivkaEntityType::Hashtag, HashtagPattern(), IsHashtag);
	collect(PrivkaEntityType::Crypto, CryptoPattern(), any);
	collect(PrivkaEntityType::Phone, PhonePlusPattern(), IsPhone);
	collect(PrivkaEntityType::Phone, PhoneGroupedPattern(), IsPhone);

	return result;
}

} // namespace Data
