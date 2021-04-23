/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/discord.h>
#include <dpp/event.h>
#include <string>
#include <iostream>
#include <fstream>
#include <dpp/discordclient.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dpp { namespace events {

using namespace dpp;

/**
 * @brief Handle event
 * 
 * @param client Websocket client (current shard)
 * @param j JSON data for the event
 * @param raw Raw JSON string
 */
void voice_state_update::handle(DiscordClient* client, json &j, const std::string &raw) {

	json& d = j["d"];
	dpp::voice_state_update_t vsu(client, raw);
	vsu.state = dpp::voicestate().fill_from_json(&d);

	/* Update guild voice states */
	dpp::guild* g = dpp::find_guild(vsu.state.guild_id);
	if (g) {
		if (vsu.state.channel_id == 0) {
			auto ve = g->voice_members.find(vsu.state.user_id);
			if (ve != g->voice_members.end()) {
				g->voice_members.erase(ve);	
			}
		} else {
			g->voice_members[vsu.state.user_id] = vsu.state;
		}
	}

	if (vsu.state.user_id == client->creator->me.id)
	{
		std::lock_guard<std::mutex> lock(client->voice_mutex);
		auto v = client->connecting_voice_channels.find(vsu.state.guild_id);
		/* Check to see if we have a connection to a voice channel in progress on this guild */
		if (v != client->connecting_voice_channels.end()) {
			v->second->session_id = vsu.state.session_id;
			if (v->second->is_ready() && !v->second->is_active()) {
				v->second->connect(vsu.state.guild_id);
			}
		}
	}

	if (client->creator->dispatch.voice_state_update) {
		client->creator->dispatch.voice_state_update(vsu);
	}
}

}};