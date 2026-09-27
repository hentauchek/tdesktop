/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_privka.h"

#include "settings/settings_common_session.h"

#include "data/data_privka.h"
#include "lang/lang_keys.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "ui/widgets/checkbox.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/vertical_list.h"
#include "window/window_session_controller.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

const auto kKeywordPrivka = u"privka"_q;

void AddFeatureCheckbox(
		SectionBuilder &builder,
		Data::PrivkaFeature feature,
		int index) {
	const auto id = u"privka/feature/"_q + QString::number(index);
	const auto title = Data::PrivkaFeatureTitle(feature);
	const auto checkbox = builder.addCheckbox({
		.id = id,
		.title = rpl::single(title),
		.checked = Data::PrivkaEnabled(feature),
		.keywords = { kKeywordPrivka, title.toLower() },
	});
	if (checkbox) {
		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Data::SetPrivkaEnabled(feature, checked);
		}, builder.container()->lifetime());
	}
}

void AddEntityCheckbox(
		SectionBuilder &builder,
		Data::PrivkaEntityType type,
		int index) {
	const auto id = u"privka/extract/"_q + QString::number(index);
	const auto title = Data::PrivkaEntityTypeTitle(type);
	const auto checkbox = builder.addCheckbox({
		.id = id,
		.title = rpl::single(title),
		.checked = Data::PrivkaEntityEnabled(type),
		.keywords = { kKeywordPrivka, title.toLower() },
	});
	if (checkbox) {
		checkbox->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			Data::SetPrivkaEntityEnabled(type, checked);
		}, builder.container()->lifetime());
	}
}

void BuildPrivkaSection(SectionBuilder &builder) {
	builder.addSubsectionTitle(tr::lng_privka_features_title());
	const auto features = Data::PrivkaFeatures();
	for (auto i = 0; i != int(features.size()); ++i) {
		AddFeatureCheckbox(builder, features[i], i);
	}

	builder.addDivider();
	builder.addSubsectionTitle(tr::lng_privka_extract_title());
	const auto types = Data::PrivkaEntityTypes();
	for (auto i = 0; i != int(types.size()); ++i) {
		AddEntityCheckbox(builder, types[i], i);
	}
}

class Privka : public Section<Privka> {
public:
	Privka(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

};

Privka::Privka(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

rpl::producer<QString> Privka::title() {
	return tr::lng_settings_privka_title();
}

void Privka::setupContent() {
	setFocusPolicy(Qt::StrongFocus);
	setFocus();

	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	const auto buildMethod = [](
			not_null<Ui::VerticalLayout*> container,
			not_null<Window::SessionController*> controller,
			Fn<void(Type)> showOther,
			rpl::producer<> showFinished) {
		auto &lifetime = container->lifetime();
		const auto highlights = lifetime.make_state<HighlightRegistry>();

		auto builder = SectionBuilder(WidgetContext{
			.container = container,
			.controller = controller,
			.showOther = std::move(showOther),
			.isPaused = Window::PausedIn(
				controller,
				Window::GifPauseReason::Layer),
			.highlights = highlights,
		});

		BuildPrivkaSection(builder);

		std::move(showFinished) | rpl::on_next([=] {
			for (const auto &[id, entry] : *highlights) {
				if (entry.widget) {
					controller->checkHighlightControl(
						id,
						entry.widget,
						base::duplicate(entry.args));
				}
			}
		}, lifetime);
	};

	build(content, buildMethod);

	Ui::ResizeFitChild(this, content);
}

const auto kMeta = BuildHelper({
	.id = Privka::Id(),
	.parentId = MainId(),
	.title = &tr::lng_settings_privka_title,
	.icon = &st::menuIconLock,
}, [](SectionBuilder &builder) {
	BuildPrivkaSection(builder);
});

} // namespace

Type PrivkaId() {
	return Privka::Id();
}

} // namespace Settings
