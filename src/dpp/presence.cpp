#include <dpp/presence.h>
#include <dpp/discordevents.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dpp {

presence::presence() : guild_id(0), user_id(0), flags(0)
{
}

presence::~presence() {
}

presence& presence::fill_from_json(nlohmann::json* j) {
	guild_id = SnowflakeNotNull(j, "guild_id");
	user_id = SnowflakeNotNull(&((*j)["user"]), "id");

	auto f = j->find("client_status");
	if (f != j->end()) {

		bool update_desktop = false, update_web = false, update_mobile = false;
		std::string desktop, mobile, web;

		if (f->find("desktop") != f->end()) {
			desktop = StringNotNull(&((*j)["client_status"]), "desktop");
			update_desktop = true;
		}
		if (f->find("mobile") != f->end()) {
			mobile = StringNotNull(&((*j)["client_status"]), "mobile");
			update_mobile = true;
		}
		if (f->find("web") != f->end()) {
			web = StringNotNull(&((*j)["client_status"]), "web");
			update_web = true;
		}

		if (update_desktop) {
			flags &= PF_CLEAR_DESKTOP;
			if (desktop == "online")
				flags |= p_desktop_online;
			else if (desktop == "idle")
				flags |= p_desktop_idle;
			else if (desktop == "dnd")
				flags |= p_desktop_dnd;
		}

		if (update_mobile) {
			flags &= PF_CLEAR_MOBILE;
			if (mobile == "online")
				flags |= p_mobile_online;
			else if (mobile == "idle")
				flags |= p_mobile_idle;
			else if (mobile == "dnd")
				flags |= p_mobile_dnd;
		}

		if (update_web) {
			flags &= PF_CLEAR_WEB;
			if (web == "online")
				flags |= p_web_online;
			else if (web == "idle")
				flags |= p_web_idle;
			else if (web == "dnd")
				flags |= p_web_dnd;
		}
	}

	if (j->find("status") != j->end()) {
		flags &= PF_CLEAR_STATUS;
		std::string main = StringNotNull(j, "status");
		if (main == "online")
			flags |= p_status_online;
		else if (main == "idle")
			flags |= p_status_idle;
		else if (main == "dnd")
			flags |= p_status_dnd;
	}


	if (j->find("activities") != j->end()) {
		activities.clear();
		for (auto & act : (*j)["activities"]) {
			activity a;
			a.name = StringNotNull(&act, "name");
			a.state = StringNotNull(&act, "state");
			a.type = (activity_type)Int8NotNull(&act, "type");
			a.url = StringNotNull(&act, "url");
			a.created_at = Int64NotNull(&act, "created_at");
			if (act.find("timestamps") != act.end()) {
				a.start = Int64NotNull(&(act["timestamps"]), "start");
				a.end = Int64NotNull(&(act["timestamps"]), "end");
			}
			a.application_id = SnowflakeNotNull(&act, "application_id");
			a.flags = Int8NotNull(&act, "flags");
	
			activities.push_back(a);
		}
	}

	return *this;
}

std::string presence::build_json() const {
	std::map<presence_status, std::string> status_name_mapping = {
		{ps_online, "online"},
		{ps_offline, "offline"},
		{ps_idle, "idle"},
		{ps_dnd, "dnd"}
	};
	json j({
		{ "status", status_name_mapping[status()] },
		{ "since", 1 },
		{ "afk", false }
	});
	if (activities.size()) {
		j["game"] = json({
			{ "name", activities[0].name },
			{ "type", activities[0].type }
		});
	}

	return j.dump();
}

presence_status presence::desktop_status() const {
	return (presence_status)((flags >> PF_SHIFT_DESKTOP) & PF_STATUS_MASK);
}

presence_status presence::web_status() const {
	return (presence_status)((flags >> PF_SHIFT_WEB) & PF_STATUS_MASK);
}

presence_status presence::mobile_status() const {
	return (presence_status)((flags >> PF_SHIFT_MOBILE) & PF_STATUS_MASK);
}

presence_status presence::status() const {
	return (presence_status)((flags >> PF_SHIFT_MAIN) & PF_STATUS_MASK);
}

};