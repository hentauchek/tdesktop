/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtCore/QString>

#include <vector>

namespace Data {

enum class PrivkaFeature {
	AntiDelete,
	ManualRead,
	KeepTtlMessages,
	NoStoriesRead,
	SaveStories,
	CopyOnDoubleClick,
};

enum class PrivkaEntityType {
	Phone,
	Username,
	Collectible,
	Crypto,
	Hashtag,
	Url,
};

struct PrivkaEntity {
	PrivkaEntityType type = PrivkaEntityType::Url;
	QString value;
};

[[nodiscard]] const std::vector<PrivkaFeature> &PrivkaFeatures();

[[nodiscard]] bool PrivkaEnabled(PrivkaFeature feature);

void SetPrivkaEnabled(PrivkaFeature feature, bool enabled);

[[nodiscard]] QString PrivkaFeatureTitle(PrivkaFeature feature);

[[nodiscard]] const std::vector<PrivkaEntityType> &PrivkaEntityTypes();

[[nodiscard]] bool PrivkaEntityEnabled(PrivkaEntityType type);

void SetPrivkaEntityEnabled(PrivkaEntityType type, bool enabled);

[[nodiscard]] QString PrivkaEntityTypeTitle(PrivkaEntityType type);

[[nodiscard]] std::vector<PrivkaEntity> PrivkaEntities(const QString &text);

} // namespace Data
