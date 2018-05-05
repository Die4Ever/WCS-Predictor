//g++ sim.cpp -o3 -I /home/rcarro/Dropbox/MyHeaders/ -std=c++11 -lpthread -lmysqlclient -o predict.exe
//g++ sim.cpp -o3 -I /home/rcarro/Dropbox/MyHeaders/ -std=c++11 -lpthread -lmysqlclient -o predict.exe && nice -n 10 ./predict.exe

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <string.h>
#include <stdio.h>
#include <random>
#include <algorithm>
#include <time.h>
#include <sstream>
#include <fstream>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <array>
#include <bitset>
using namespace std;
#define RAYHEADERSMAIN
#define VERSION "0.574"
#define DEBUGFILENAME "debugout-" VERSION ".txt"
#define SIMULATOR 1
#ifdef _DEBUG
#define DEBUGMULTITHREAD
#define NOSTACK
#define DEBUGLEVEL INFO//CALLSRETURNS//ERR
#define DEBUGFILELEVEL INFO
#else
#define DEBUGLEVEL WARN//CALLSRETURNS//ERR
#define DEBUGFILELEVEL WARN
#endif
#include <RayCGI2.h>
#include <RayRand.h>
#include <sockets4.h>
#include "../sc2_common.h"

const size_t BLOCK_SIZE = 500000*10;//1 mil? 500k was good before, but maybe I need the extra to properly do awesome branches?
const size_t TOTAL_SAMPLES = 25000000;//25000000;
const size_t NUM_BLOCKS = TOTAL_SAMPLES/BLOCK_SIZE -1;
const int USE_ALIGULAC=1;
const bool BENCHMARK = false;
bool AFK = false;//this is set to true via command line
#ifdef _DEBUG
const int THREADS = 1;
#else
const int THREADS = 8;
#endif
string description;

class badplayer : public rayexception
{
public:
	badplayer(string s DEBUGARGSDEF) : rayexception(("badplayer: " + s).c_str() DEBUGARGS)
	{
	}
};

string HttpRequest(string url, bool readcache, bool unescape, uint re=0)
{
	if(url.length()<3) DEBUGOUTSTACK(FATAL, url, "url is too short!");
	if(BENCHMARK) readcache=true;
	//if(BDEBUG) readcache=true;
	//readcache=true;

	if(re>5) {
		cerr <<"Failed "<<re<<" times to retreive "<<url<<"\n";
		if(readcache==false) {
			cerr <<"fuck it, use the cache\n";
			readcache=true;
		} else return "";
		//return "";
	}

	string cachename=url;
	cachename=str_replace(cachename.c_str(), "/","-");
	cachename=str_replace(cachename.c_str(), ":","-");
	cachename=str_replace(cachename.c_str(), "?","-");
	cachename+=".txt";
	cachename="cache/"+cachename;

	if (readcache) {
		string resp;
		ifstream cfile(cachename, ios::binary);
		while (cfile.good()) {
			char buff[1024*32];
			cfile.read(buff, 1024*32);
			resp+=string(buff, cfile.gcount());
		}
		if(resp.length()) {
			//cerr << "cache hit "<<cachename<<"\n";
			return resp;
		}
		cerr << "cache miss "<<cachename<<"\n";
		DEBUGOUTSTACK(ERR, url, "");
	}
	else {
		cerr << "not caching " << cachename << "\n";
	}
	RaySleep(1000*1000*4);

	//cerr << "\n\tHttpRequest("<<url<<")\n\n";
	static string cookie = "";
	BasicSocket sock;
	vector<string> urlsplit = SuperSplit(url.c_str(), "http://%/%");

	sock.Connect(urlsplit[0].c_str(), 80);
	string send = "GET /" + urlsplit[1] + " HTTP/1.0\r\nHost: " + urlsplit[0] + "\r\nConnection: close\r\nUser-Agent: Die4Ever Stats\r\n";
	if (cookie.length() > 0)
		send += cookie;
	send += "\r\n";
	sock.Send(send.c_str(), (int)send.length());

	string response;
	while (sock.Status == sock.Connected)
	{
		char buff[65536];
		if (sock.Recv(buff, 65535) > 0)
		{
			buff[sock.iLastRecv] = '\0';
			response += buff;
		}
	}
	if (unescape == true)
		response = UnescapeHtml(response);
	if (strstr(response.substr(0, 500).c_str(), "HTTP/1.1 503 Service Unavailable") && strstr(response.c_str(), "Set-Cookie:"))
	{
		//cout << response << "\n\n";
		vector<string> split = SuperSplit_R(response.c_str(), "Set-Cookie: %;");
		cookie = "Cookie: ";
		for (uint i = 0; i < split.size() - 1; i++)
			cookie += split[i] + "; ";
		cookie += "\r\n";
		//cout << cookie;
		response = HttpRequest(url, readcache, unescape);
	}
	if (strstr(response.substr(0, 50).c_str(), "HTTP/1.1 302") || strstr(response.substr(0, 50).c_str(), "HTTP/1.1 301"))
	{
		//cout << response << "\n";
		vector<string> split = SuperSplit(response.c_str(), "Location: %\r\n");
		string newurl = split[0];
		if (newurl[0] == '/')
			newurl = string("http://") + urlsplit[0] + split[0];
		//cout << split[0] << "\n";
		response = HttpRequest(newurl, readcache, unescape);
	}
	//cout << "done HttpRequest("<<url<<")\n";
	if(response=="") {
		RaySleep(1000*1000*15);
		cerr << "retrying "<<url<<"...\n";
		return HttpRequest(url, readcache, unescape, re+1);
	}

	{
		ofstream cfile(cachename, ios::binary);
		cfile.write(response.c_str(), response.length());
	}
	return response;
}

vector<string> HttpRequests(vector<string> &urls, bool readcache, bool unescape)
{
	vector<string> ret(urls.size());
	atomic<int> threads;
	threads = 0;
	for(uint i=0;i<urls.size();i++) {
		string &turl=urls[i];
		string &tret=ret[i];
		while (threads>0) RaySleep(1000);
		threads++;
		//thread([&threads, &turl, &tret, i, readcache, unescape]() {
			tret = HttpRequest(turl, readcache, unescape);
			threads--;
		//}).detach();
	}
	while (threads>0) RaySleep(1000);
	return ret;
}

#include "players.h"

array<Player*, 3> TripleTieBreaker(uint a, uint b, uint c, RayRand64 &rng DEBUGARGSDEF);

#include "wcs_points_counter.h"
#include "simulation.h"
#include "sim_modules.h"
#include "wcs2015.h"

array<Player*, 3> TripleTieBreaker(uint a, uint b, uint c, RayRand64 &rng DEBUGARGSDEF)
{
	RoundRobin<3, 3> round_robin;
	round_robin.players[0] = &GetPlayerById(a DEBUGARGS);
	round_robin.players[1] = &GetPlayerById(b DEBUGARGS);
	round_robin.players[2] = &GetPlayerById(c DEBUGARGS);
	return round_robin.PredictNoOldResults(5, rng DEBUGARGS);
}

class PlayerEvents
{
public:
	vector<EventCounter> events[EVENTS_HASH_SIZE];//basic hash table

	EventCounter GetEventCounter(Event &e)
	{
		uint slot = e.Hash() % EVENTS_HASH_SIZE;
		for (size_t i = 0; i < events[slot].size(); i++) {
			if (events[slot][i].SortComp(e) == 0) {
				return events[slot][i];
			}
		}
		//throw rayexception("GetEventCounter failed to find event" DEBUGARGS);
		return EventCounter();
	}

	void CountEvent(Event &e, bool hit, uint incr)
	{
		uint slot = e.Hash() % EVENTS_HASH_SIZE;
		for (size_t i = 0; i < events[slot].size(); i++) {
			if (events[slot][i].SortComp(e) == 0) {
				if (hit) {
					events[slot][i].hits+=incr;
				} else {
					events[slot][i].misses+=incr;
				}
				return;
			}
		}

		events[slot].push_back(EventCounter(e));
		if (hit) {
			events[slot].back().hits+=incr;
		} else {
			events[slot].back().misses+=incr;
		}
	}

	void AddEvent(EventCounter &e)
	{
		uint slot = e.Hash() % EVENTS_HASH_SIZE;
		for (size_t i = 0; i < events[slot].size(); i++) {
			if (events[slot][i].SortComp(e) == 0) {
				assert(0);
				return;
			}
		}

		events[slot].push_back(e);
	}
};

class PlayerEventGroups
{
public:
	uint p_id;
	vector<EventGroup> eventgroups[EVGS_HASH_SIZE];//basic hash table

	void shrink_to_fit()
	{
		for (uint i = 0; i < EVGS_HASH_SIZE; i++) {
			eventgroups[i].shrink_to_fit();
		}
	}

	void Add(EventGroup evg)
	{
		uint slot = evg.Hash() % EVGS_HASH_SIZE;
		for (size_t i = 0; i < eventgroups[slot].size(); i++) {
			if (evg.SortComp(eventgroups[slot][i]) == 0)
				return;
		}
		eventgroups[slot].push_back(evg);
	}
	
	void Add(EventGroup evg, bool made_top, uint incr)
	{
		uint slot = evg.Hash() % EVGS_HASH_SIZE;
		for (size_t i = 0; i < eventgroups[slot].size(); i++) {
			if (evg.SortComp(eventgroups[slot][i]) == 0) {
				if(made_top) { eventgroups[slot][i].hits+=incr;
				} else { eventgroups[slot][i].misses+=incr; }
				return;
			}
		}
		eventgroups[slot].push_back(evg);
		if(made_top) { eventgroups[slot].back().hits+=incr;
		} else { eventgroups[slot].back().misses+=incr; }
	}
};

class EventGroupsPlayers
{
public:
	//can later change this to a hash table or bloom filter
	vector<PlayerEventGroups> playereventgroups;//simple lookup table on aligulac_id

	EventGroupsPlayers() : playereventgroups(MAX_PLAYER_ID)//aligulac ids seem to go up to about only 5000
	{
	}

	void shrink_to_fit(uint p_id)
	{
		playereventgroups[p_id].shrink_to_fit();
	}

	void Add(EventGroup evg, uint p_id)
	{
		playereventgroups[p_id].Add(evg);
	}
	
	void Add(EventGroup evg, uint p_id, bool made_top, uint incr)
	{
		playereventgroups[p_id].Add(evg, made_top, incr);
	}
};

string Simulation::Output()
{
	string out = "\n-------------------------------------------\n";
	out += "Top Players\n";
	uint top_num = 25;
	for (uint i = 0; i < player_points.size() && i<top_num; i++) {
		out += "#" + ToString(i + 1) + " - " + players[player_points[i].first].name.ToString() + " with " + ToStringCommas(player_points[i].second) + " points\n";
	}
	out += "[spoiler=Event Listing]Note that this list does not include every event for all tournaments without confirmed player listings.\n";
	auto evs = events;
	sort(evs.begin(), evs.end(), [](const Event &a, const Event &b) {
		return a.t_id < b.t_id;
	});
	for (auto &e : evs) {
		bool is_top_player = false;
		for (uint i = 0; i < player_points.size() && i < top_num; i++) {
			if (player_points[i].first == e.p_id) is_top_player = true;
		}
		if (is_top_player == false) continue;
		if (e.placing == QUALIFY_BLIZZCON) continue;

		if (e.placing < SPECIAL_EVENT) {
			out += players[e.p_id].name.ToString() + " got " + ToString((int)e.placing) + " place in " + TournamentNames[(int)e.t_id] + "\n";
		}
		else {
			out += players[e.p_id].name.ToString();
			uint tid = 0;
			uint roundof = 0;
			for (auto &u : upcoming_matches) {
				if (u.id == (uint)e.t_id) {
					tid = u.t_id;
					roundof = u.round;
				}
			}
			if (e.placing == UPCOMING_WIN) out += " won upcoming match in " + TournamentNames[tid] + " round of " + ToString(roundof) + "\n";
			else if (e.placing == UPCOMING_LOSE) out += " lost upcoming match in " + TournamentNames[tid] + " round of " + ToString(roundof) + "\n";
			//else out += " other special event, placing==" + ToString((int)e.placing) + ", t_id==" + ToString((int)e.t_id) + "\n";
		}
	}
	out += "[/spoiler]\n";
	out += "\n-------------------------------------------\n";
	return out;
}

void insert_upcoming_matches(MysqlCon &con, unsigned __int64 sim_id, vector<UpcomingMatch> &upcoming_matches)
{
	if (BENCHMARK || BDEBUG) return;
	string query = "insert into upcoming_matches(sim_id,played_on,t_id,round,season,upm_id,p_id) values ";
	string values;
	for (uint i = 0; i<upcoming_matches.size(); i++) {
		auto &upm = upcoming_matches[i];
		for (uint p = 0; p<8; p++) {
			if (upm.players[p] == 0) continue;
			values += "(" + ToString(sim_id) + ",from_unixtime(" + ToString(upm.timestamp) + ")," + ToString(upm.t_id) + "," +
				ToString(upm.round) + ",0," + ToString(upm.id) + "," + ToString(upm.players[p]) + "),";
		}
	}
	if (values.length()) {
		query += values.substr(0, values.length() - 1);
		con.update(query.c_str() DEBUGARGS);
	}
}

void insert_previous_matches(MysqlCon &con, unsigned __int64 sim_id, vector<PreviousMatch> &prev_matches)
{
	if (BENCHMARK || BDEBUG) return;
}

class SimBranch
{
	EventGroupsPlayers event_groups;
	PointsCounter points_counter;
	vector<unique_ptr<SimModule> > mods;
	//array< pair<uint, uint>, MAX_PLAYER_ID > players_times_top;
	array< uint, MAX_PLAYER_ID > players_times_top;
	uint last_samples;

protected:

	bool _MatchSim(Simulation &sim)
	{
		return true;
	}

	void DisplayStats(uint samps, array<uint,MAX_PLAYER_ID> &players_times_top)//vector< pair<uint,uint> > &players_times_top)
	{
		return;
		cout << "\n\nsamps=="<<samps<<"\n";
		for (uint i = 0; i < players_times_top.size();i++) {
			auto p = players_times_top[i];
			if(players[i].player_id==0 || p==0) continue;
			string name=players[i].name.ToString();
			double prob=(double)p / (double)samps *100.0;
			if(prob<5.0) continue;
			cerr << name<<" at "<<prob<<"% chances\n";
		}
	}

public:
	uint type;

	size_t sim_id;
	size_t base_sim_id;
	string branch_desc;

	SimBranch(string Description)
	{
		last_samples = 0;
		sim_id = 0;
		base_sim_id = 0;
		//make mods
		mods.push_back(unique_ptr<ForeignHopeHandler>(new ForeignHopeHandler));
		mods.push_back(unique_ptr<FinalFacingStats>(new FinalFacingStats));
		branch_desc = Description;
		type = 1;
	}

	SimBranch(SimBranch &&o) : mods(std::move(o.mods)), branch_desc(o.branch_desc)
	{
		event_groups = o.event_groups;
		points_counter = o.points_counter;
		players_times_top = o.players_times_top;
		last_samples = o.last_samples;
		type = o.type;
		sim_id = o.sim_id;
		base_sim_id = o.base_sim_id;
	}

	virtual ~SimBranch()
	{
	}

	void init_db_base(MysqlCon &con, size_t BaseSimID DEBUGARGSDEF)
	{
		//if (BDEBUG) return;
		base_sim_id = BaseSimID;
		sim_id = (uint)con.insert(("insert into sims set created=now(), size=0, first_season=1, last_season=3, includes_full_year=1, use_aligulac=" + ToString(USE_ALIGULAC) + ", type="+ToString(type)+", base_sim_id=0").c_str() DEBUGARGS);
		if (description.length()) {
			con.insert(("insert into update_events set created=(select created from sims where sim_id=" + ToString((uint)sim_id) + "),t_id=0,p_id=0,description='" + con.escape(description, 65535) + "'").c_str() DEBUGARGS);
		}
		cerr << "base got sim_id ==" << sim_id << "\n";

		insert_upcoming_matches(con, sim_id, upcoming_matches);
		insert_previous_matches(con, sim_id, prev_matches);
	}

	void init_db(MysqlCon &con, size_t BaseSimID DEBUGARGSDEF)
	{
		//if (BDEBUG) return;
		base_sim_id = BaseSimID;
		auto res = con.select(("select created+interval 1 second from sims where sim_id=" + ToString((uint)base_sim_id)).c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		string base_created;
		if (num_rows) {
			auto row = con.fetch_row(res);
			base_created = row->ToString();
		}
		sim_id = (uint)con.insert(("insert into sims set created='" + con.escape(base_created, 50) + "', size=0, first_season=1, last_season=3, includes_full_year=1, use_aligulac=" + ToString(USE_ALIGULAC) + ", type="+ToString(type)+", base_sim_id=" + ToString(base_sim_id)).c_str() DEBUGARGS);
		init_db_branch(con DEBUGARGS);
		cerr << "branch got sim_id == " << sim_id << "\n";

		insert_upcoming_matches(con, sim_id, upcoming_matches);
		insert_previous_matches(con, sim_id, prev_matches);
	}

	virtual void init_db_branch(MysqlCon &con DEBUGARGSDEF)
	{
		con.insert(("insert into branches set sim_id=" + ToString((uint)sim_id) + ",description='" + con.escape(branch_desc, 255) + "', p_id=0,t_id=0,placing=0").c_str() DEBUGARGS);
	}

	void init_save(MysqlCon &con DEBUGARGSDEF)
	{
		//if (BDEBUG) return;
		for (auto &m : mods) {
			m->Save(con, sim_id DEBUGARGS);
		}
		points_counter.Save(con, sim_id DEBUGARGS);
		con.update(("update sims set size=size+" + ToString(last_samples) + " where sim_id=" + ToString(sim_id)).c_str() DEBUGARGS);
		insert_update_players_chances(con, sim_id);
		insert_event_groups(con, sim_id, event_groups DEBUGARGS);
	}

	void save(MysqlCon &con DEBUGARGSDEF)
	{
		if (BDEBUG) return;
		for (auto &m : mods) {
			m->Save(con, sim_id DEBUGARGS);
		}
		points_counter.Save(con, sim_id DEBUGARGS);
		con.update(("update sims set size=size+" + ToString(last_samples) + " where sim_id=" + ToString(sim_id)).c_str() DEBUGARGS);
		insert_update_players_chances(con, sim_id);
		update_event_groups(con, sim_id, event_groups DEBUGARGS);
	}

	void insert_update_players_chances(MysqlCon &con, unsigned __int64 sim_id)
	{
		auto s = GetMilliCount();
		string sim_id_string = ToString(sim_id);
		string query = "insert into players_chances (p_id,sim_id,season,times_top) values ";
		for (auto &p : players) {
			if (p.player_id == 0) continue;
			uint times_top = players_times_top[p.player_id];// p.times_top;
			query += "(" + ToString(p.player_id) + "," + sim_id_string + ",0," + ToString(times_top) + "),";
		}
		query = query.substr(0, query.length() - 1);
		query += " on duplicate key update times_top=times_top+values(times_top)";
		con.update(query.c_str() DEBUGARGS);
		//cerr << "insert_update_players_chances took " << GetMilliSpan(s) << " ms\n";
	}

	void insert_event_groups(MysqlCon &con, unsigned __int64 sim_id, EventGroupsPlayers &event_groups DEBUGARGSDEF)
	{
		unsigned __int64 evg_id = (sim_id * 4) * 10000;
		auto s = GetMilliCount();

		string queryA = "insert into event_groups (evg_id, p_id, sim_id, season, hits, misses, total, type, size) values ";
		string queryB = "insert into events (evg_id, type, p_id, t_id, placing) values ";

		string valuesA;
		string valuesB;

		for (auto &p : players) {
			if (p.player_id == 0) continue;
			auto &player_evgs = event_groups.playereventgroups[p.player_id];

			for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
				for (size_t eg = 0; eg < player_evgs.eventgroups[i].size(); eg++) {
					auto & evg = player_evgs.eventgroups[i][eg];
					evg.evg_id = ++evg_id;
					size_t size = evg.size();
					valuesA += "(" + ToString(evg.evg_id) + "," + ToString(p.player_id) + "," + ToString(sim_id) + ",0,"
						+ ToString(evg.hits) + "," + ToString(evg.misses) + "," + ToString(evg.hits + evg.misses) + ",1," + ToString(size) + "),";
					for (size_t e = 0; e < size; e++) {
						valuesB += "(" + ToString(evg.evg_id) + ",1," + ToString(evg.events[e].p_id) + "," + ToString((int)evg.events[e].t_id) + "," + ToString((int)evg.events[e].placing) + "),";
					}
				}
			}
		}
		if (valuesA.length() && valuesB.length()) {
			queryA += valuesA.substr(0, valuesA.length() - 1);
			queryB += valuesB.substr(0, valuesB.length() - 1);

			con.update(queryA.c_str() DEBUGARGS);
			con.update(queryB.c_str() DEBUGARGS);
		}
		//cerr << "insert_event_groups took " << GetMilliSpan(s) << " ms\n";
	}

	void update_event_groups(MysqlCon &con, unsigned __int64 sim_id, EventGroupsPlayers &event_groups DEBUGARGSDEF)
	{
		auto s = GetMilliCount();
		string query = "insert into event_groups (evg_id, p_id, sim_id, season, hits, misses, total, type, size) values ";
		string values;

		for (auto &p : players) {
			if (p.player_id == 0) continue;

			auto &player_evgs = event_groups.playereventgroups[p.player_id];
			for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
				for (size_t eg = 0; eg < player_evgs.eventgroups[i].size(); eg++) {
					auto & evg = player_evgs.eventgroups[i][eg];
					size_t size = evg.size();
					values += "(" + ToString(evg.evg_id) + "," + ToString(p.player_id) + "," + ToString(sim_id) + ",0,"
						+ ToString(evg.hits) + "," + ToString(evg.misses) + "," + ToString(evg.hits + evg.misses) + ",1," + ToString(size) + "),";
				}
			}
		}

		if (values.length()) {
			query += values.substr(0, values.length() - 1);
			query += " on duplicate key update hits=hits+values(hits), misses=misses+values(misses), total=total+values(total)";
			con.update(query.c_str() DEBUGARGS);
		}
		//cerr << "update_event_groups took " << GetMilliSpan(s) << " ms\n";
	}

	void ConsumePointsCounter(const PointsCounter &a)
	{
		points_counter.ConsumeCounter(a);
	}
	
	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		if(typeid(*this)==typeid(SimBranch)) {
			throw rayexception("SimBranch shouldn't be using filter" DEBUGARGS);
			sims=origsims;
			return;
		}
		for(auto &s : origsims) if(_MatchSim(s)) sims.push_back(s);
	}

	void PurgeInsignificantEvents(vector<Simulation> &sims DEBUGARGSDEF)
	{
		size_t dumped=0;
		size_t kept=0;
		auto start=GetMilliCount();
		auto samples=sims.size();
		double dsamps=(double)samples;
		PlayerEvents events;
		for(size_t s=0;s<samples;s++) {
			Simulation &sim = sims[s];
			for(size_t i=0;i<sim.events.size();i++) {
				Event &ev = sim.events[i];
				events.CountEvent(ev, true, 1);
			}
		}
		for(size_t s=0;s<samples;s++) {
			Simulation &sim = sims[s];
			decltype(sim.events) tevents;
			for(size_t i=0;i<sim.events.size();i++) {
				Event &ev = sim.events[i];
				auto evc = events.GetEventCounter(ev);
				//evc.hits/=EVENTS_DIVISOR;
				//evc.misses/=EVENTS_DIVISOR;
				if( evc.p_id>0 && (double)evc.hits/dsamps >= MINGLOBALEVENTRESOLUTION && (double)evc.hits/dsamps <= (1.0-MINGLOBALEVENTRESOLUTION) ) {
					tevents.push_back(ev);
					kept++;
				} else dumped++;
			}
			sim.events=tevents;
			sim.RecalcBloomFilter();//these should still be sorted
		}
		auto took=GetMilliSpan(start);
		cerr << "PurgeInsignificantGlobalEvents took "<<took<<"ms, dumped "<<((double)dumped/(double)(dumped+kept))*100.0<<"%, avg of "<<(dumped/samples)<<" events per sample ("<<dumped<<")\n";
	}

	void AddImportantEvents(vector<Simulation> &sims)
	{
		array<UpcomingMatch,256> upms;
		for(auto &upm : upcoming_matches) {
			upms[upm.id]=upm;
		}
		cerr << "manually adding important events\n";
		for(uint i=0;i<sims.size();i++) {
			auto &sim=sims[i];
			for(auto &e : sim.events) {
				if(e.placing==UPCOMING_WIN || e.placing==UPCOMING_LOSE || e.placing==1 || e.t_id==100) {
					if(e.placing==UPCOMING_WIN || e.placing==UPCOMING_LOSE) {
						bool found=false;
						auto &upm = upms[(int)e.t_id];
						for(auto p : upm.players) {
							if(p==(uint)e.p_id) {
								found=true;
								break;
							}
						}
						if(found==false) continue;
					}
					EventGroup evg;
					evg.AddEvent(EventCounter(e));
					evg.hits=(uint)sims.size()*2;
					evg.misses=0;
					event_groups.Add(evg, (uint)e.p_id);
				}
			}
		}
	}

	void CountEventsPlayer(uint times_top, vector<bool> &made_tops, vector<Simulation> &sims, PlayerEvents &events, uint pid DEBUGARGSDEF)
	{
		double prob=(double)times_top / (double)sims.size();
		uint TBLOCK_SIZE = BLOCK_SIZE;// (uint)sims.size();
		uint incr=(uint)std::max<uint>(1u,(uint)(sims.size()*EVENTS_DIVISOR/TBLOCK_SIZE));
		size_t end=sims.size() / incr;
		/*if(sims.size() > BLOCK_SIZE/4) {
			incr=EVENTS_DIVISOR;
			end=sims.size() / EVENTS_DIVISOR;
		}*/
		for (size_t s = 0; s < end; s++) {
			Simulation &sim = sims[s];
			bool made_top = made_tops[s];

			for(auto &ev : sim.events) {
				events.CountEvent(ev, made_top, incr);
			}
		}
	}

	void RemoveIrrelevantEvents(uint sims, uint times_top, PlayerEvents &events, uint pid DEBUGARGSDEF)
	{
		uint times_failed = sims - times_top;
		double prob = (double)times_top / (double)sims;

		vector<EventCounter> ecs;

		size_t total = 0;
		for (size_t a = 0; a < EVENTS_HASH_SIZE; a++) {
			total += events.events[a].size();
			ecs.insert(ecs.end(), events.events[a].begin(), events.events[a].end());
			events.events[a].clear();
		}

		auto &egs = ecs;
		for (uint a = 0; a < egs.size(); a++) {
			for (uint b = a + 1; b < egs.size(); b++) {
				if (egs[a].hits == egs[b].hits && egs[a].misses == egs[b].misses) {
					double scoreA = egs[a].PurgeScoreEvent(sims, prob, pid);
					double scoreB = egs[b].PurgeScoreEvent(sims, prob, pid);
					uint o = b;//favor the event found first slightly
					if (scoreB > scoreA) o = a;
					if ( (egs[o].placing == 1 || egs[o].placing>250) && egs[o].p_id==pid) continue;
					egs[o] = egs.back();
					egs.pop_back();
					a--;
					break;
				}
			}
		}

		sort(ecs.begin(), ecs.end(), [sims, prob, pid](const EventCounter &a, const EventCounter &b) {
			double scoreA = a.PurgeScoreEvent(sims, prob, pid);
			double scoreB = b.PurgeScoreEvent(sims, prob, pid);
			return scoreA > scoreB;
		});

		if (ecs.size() > MAX_PLAYER_EVENTS) {
			ecs.resize(MAX_PLAYER_EVENTS, ecs[0]);
		}
		while (ecs.size() && ecs.back().PurgeScoreEvent(sims, prob, pid) < MINEVENTSCORE / 4.0) {
			ecs.pop_back();
		}

		//cerr << players[pid].name<<" has "<<ecs.size()<<" events\n";

		for (auto &ec : ecs) {
			events.AddEvent(ec);
		}

		/*total = 0;
		for (size_t a = 0; a < EVENTS_HASH_SIZE; a++) {
			total += events.events[a].size();
		}*/
	}

	void GroupEvents(vector<Simulation> &sims, uint times_top, vector<bool> &made_tops, PlayerEvents &events, uint pid, EventGroupsPlayers &event_groups, RayRand64 &rng DEBUGARGSDEF)
	{
		uint samps = (uint)sims.size();
		uint times_failed = samps - times_top;
		double prob = (double)times_top / (double)samps;

		if (prob < MINCHECKEVENTS) {
			return;
		}
		double avg_score = 0.0;
		double min_score_mult=0.75;
		uint incr = (uint)std::max<uint>(1u,samps*GROUP_EVENTS_DIVISOR/BLOCK_SIZE);
		for (uint s = 0; s < samps / incr; s++) {
			bool made_top = true;//made_tops[s];//the counts get reset anyways, I don't need to waste cache on this
			EventGroup evg;
			size_t total = 0;
			total = sims[s].events.size();

			for (size_t i = 0; i < total / incr; i++) {// divided by 4? I guess the full thing is overkill?
				if (total == 0) continue;

				size_t e = rng.iRand64HQ() % total;
				auto &ev = sims[s].events[e];
				auto evc = events.GetEventCounter(ev);
				if (evc.p_id == 0) continue;

				double score = evc.ScoreEvent(samps, prob, pid);
				avg_score -= avg_score / (double)MAX_PLAYER_EVENT_GROUPS;
				avg_score += (score) / (double)MAX_PLAYER_EVENT_GROUPS;

				size_t evg_size = evg.size();
				double tevgprob=(double)(evc.hits+evc.misses)/(double)samps;
				double tscore=score;
				bool self = evc.p_id == pid;
				for(uint i=0;i<evg_size;i++) {
					tevgprob*=(double)(evg.events[i].hits+evg.events[i].misses)/(double)samps;
					tscore+=evg.events[i].PurgeScoreEvent(samps, prob, pid);
					if (evg.events[i].p_id != pid) self = false;
				}
				tscore/=((double)(evg_size+1)/2.0)+0.5;
				tscore += 999.0*((double)self);
				tevgprob += 999.0*((double)self);
				if (tevgprob*4.0 < MINEVENTPROB || tscore<avg_score*min_score_mult*log2((double)(evg_size+1+4)) ) {
					//don't do multiples with this
				} else if (evg_size>0 && evg_size < MAX_EVENTS_GROUP) {
					evg.AddEvent(evc, evg_size);
					event_groups.Add(evg, pid, made_top, (uint)(GROUP_EVENTS_DIVISOR*GROUP_EVENTS_DIVISOR*8*(evg_size+1)) );
				} else {
					evg = EventGroup();
					evg.AddEvent(evc, 0);
				}
			}
		}

		for (size_t a = 0; a < EVENTS_HASH_SIZE; a++) {
			for(auto &ev : events.events[a]) {
				EventGroup evg;
				evg.AddEvent(ev, 0);
				event_groups.Add(evg, pid, true, 1);//the counts get cleared anyways...
			}
		}
	}

	void CountEventGroupsPrelim(vector<Simulation> &sims, vector<bool> &made_tops, EventGroupsPlayers &event_groups, uint pid DEBUGARGSDEF)
	{
		auto &evgs = event_groups.playereventgroups[pid];
		size_t total = 0;
		for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
			total += evgs.eventgroups[i].size();
			for(auto &e : evgs.eventgroups[i]) {
				e.hits=0;
				e.misses=0;
			}
		}
		if (total == 0)
			return;

		uint PRELIM_COUNT_SIMS = (uint)std::max<uint>(1u, (uint)(sims.size()*PRELIM_COUNT_DIVISOR / BLOCK_SIZE));

		for (size_t s = 0; s < sims.size()/PRELIM_COUNT_SIMS; s++) {
			Simulation &sim = sims[s];
			bool made_top = made_tops[s];
			
			for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
				for (size_t e = 0; e < evgs.eventgroups[i].size(); e++) {
					auto &evg = evgs.eventgroups[i][e];
					size_t len = evg.size();
					size_t matches = 0;
					matches = sim.FindEvents(evg);
					if (matches == len) {
						if (made_top)
							evg.hits+=PRELIM_COUNT_SIMS;
						else
							evg.misses+=PRELIM_COUNT_SIMS;
					}
				}
			}
		}
	}

	void RemoveSupersLoose(EventGroup &sub, vector<EventGroup> &supes, array<vector<EventGroup>, MAX_EVENTS_GROUP> &split, uint next)
	{
		for(uint a=0;a<supes.size();a++) {
			auto r = supes[a].LooseSuperSubSet(sub);
			if(r==1) {//1 means a is the superset, kill it
				//cerr <<"a";
				for(uint n=next;n<split.size();n++) {
					RemoveSupersLoose(supes[a], split[n], split, n+1);
				}
				supes[a]=supes.back();
				supes.pop_back();
				a--;//a has now changed, start over on the same a value
				//break;
			}
		}
	}

	void RemoveSupers(vector<EventGroup> &subs, vector<EventGroup> &supes, array<vector<EventGroup>, MAX_EVENTS_GROUP> &split, uint next)
	{
		for(uint a=0;a<supes.size();a++) {
			for(uint b=0;b<subs.size();b++) {
				auto r = supes[a].SuperSubSet(subs[b]);
				if(r==1) {//1 means a is the superset, kill it
					//cerr <<"a";
					for(uint n=next;n<split.size();n++) {
						RemoveSupersLoose(supes[a], split[n], split, n+1);
					}
					supes[a]=supes.back();
					supes.pop_back();
					a--;//a has now changed, start over on the same a value
					break;
				}
			}
		}
	}

	void RemoveIrrelevantEventGroupsPrelim(uint sims, uint times_top, uint pid, EventGroupsPlayers &event_groups DEBUGARGSDEF)
	{
		double prob = (double)times_top / (double)sims;
		auto &player_evgs = event_groups.playereventgroups[pid];
		
		array<vector<EventGroup>, MAX_EVENTS_GROUP> split;
		
		size_t total = 0;
		for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
			total += player_evgs.eventgroups[i].size();
			for(auto & eg : player_evgs.eventgroups[i]) {
				split[eg.size()-1].push_back(eg);
			}
			player_evgs.eventgroups[i].clear();
		}
		if(total==0) return;//why wouldn't I do this???
		
		if(MAX_EVENTS_GROUP>=2) {
			for(uint a=0;a<split.size();a++) {
				for(uint i=a+1;i<split.size();i++) {
					RemoveSupers(split[a], split[i], split, i+1);
				}
			}
		}
		
		for(uint i=0;i<split.size();i++) {
			auto &s=split[i];
			sort(s.begin(), s.end(), [sims, prob, pid](const EventGroup &a, const EventGroup &b) {
				double scoreA = a.PurgeScoreEvent(sims, prob, pid);
				double scoreB = b.PurgeScoreEvent(sims, prob, pid);
				return scoreA > scoreB;
			});
			
			uint maxevgs=MAX_PLAYER_EVENT_GROUPS;
			if(i>1) maxevgs/=i*2;
			if (s.size() > maxevgs) {
				s.resize(maxevgs);
			}
			
			for (auto &eg : s) {
				eg.hits=0;
				eg.misses=0;
				player_evgs.Add(eg);
			}
		}
	}

	void CountEventGroups(vector<Simulation> &sims, vector<bool> &made_tops, EventGroupsPlayers &event_groups, uint pid DEBUGARGSDEF)
	{
		auto &evgs = event_groups.playereventgroups[pid];
		size_t total = 0;
		for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
			total += evgs.eventgroups[i].size();
			for(auto &e : evgs.eventgroups[i]) {
				e.hits=0;
				e.misses=0;
			}
		}
		if (total == 0)
			return;

		for (size_t s = 0; s < sims.size(); s++) {
			Simulation &sim = sims[s];
			bool made_top = false;
			if(made_tops.size()) made_top = made_tops[s];

			for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
				for (size_t e = 0; e < evgs.eventgroups[i].size(); e++) {
					auto &evg = evgs.eventgroups[i][e];
					size_t len = evg.size();
					size_t matches = 0;
					matches = sim.FindEvents(evg);
					if (matches == len) {
						if (made_top)
							evg.hits++;
						else
							evg.misses++;
					}
				}
			}
		}
	}

	void RemoveIrrelevantEventGroups(uint sims, uint times_top, uint pid, EventGroupsPlayers &event_groups DEBUGARGSDEF)
	{
		double prob = (double)times_top / (double)sims;
		auto &player_evgs = event_groups.playereventgroups[pid];

		vector<EventGroup> egs;
		array<vector<EventGroup>, MAX_EVENTS_GROUP> split;

		size_t total = 0;
		for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
			total += player_evgs.eventgroups[i].size();
			for(auto & eg : player_evgs.eventgroups[i]) {
				split[eg.size()-1].push_back(eg);
			}
			player_evgs.eventgroups[i].clear();
		}
		if(total==0) return;//why wouldn't I do this???

		if(prob>MINCHECKEVENTS && split[1].size()>0) {
			/*cerr << "\n"<<players[pid].name<<" had "<<total<<" event groups before removing irrelevants\n";
			for(uint i=0;i<split.size();i++) {
				cerr << "\t"<<players[pid].name<<" had "<<split[i].size()<<" event groups with size of "<<(i+1)<<"\n";
			}*/
		}
		if(MAX_EVENTS_GROUP>=2) {
			for(uint a=0;a<split.size();a++) {
				for(uint i=a+1;i<split.size();i++) {
					RemoveSupers(split[a], split[i], split, i+1);
				}
			}
		}

		for(auto &s : split) {
			egs.insert(egs.end(), s.begin(), s.end());
		}

		if(prob>MINCHECKEVENTS && split[1].size()>0) {
			/*cerr << players[pid].name<<" had "<<egs.size()<<" event groups after removing supers\n";
			for(uint i=0;i<split.size();i++) {
				cerr << "\t"<<players[pid].name<<" had "<<split[i].size()<<" event groups with size of "<<(i+1)<<"\n";
			}
			cerr <<"\n";*/
		}

		for (uint a = 0; a < egs.size(); a++) {
			for (uint b = a + 1; b < egs.size(); b++) {
				if (egs[a].hits == egs[b].hits && egs[a].misses == egs[b].misses) {
					double scoreA = egs[a].PurgeScoreEvent(sims, prob, pid);
					double scoreB = egs[b].PurgeScoreEvent(sims, prob, pid);
					uint o = b;//favor the evg found first slightly!
					if (scoreB > scoreA) o = a;
					if (egs[o].size() == 1 && (egs[o].events[0].placing == 1 || egs[o].events[0].placing>250) && egs[o].events[0].p_id == pid) continue;
					egs[o] = egs.back();
					egs.pop_back();
					a--;
					break;
				}
			}
		}

		sort(egs.begin(), egs.end(), [sims, prob, pid](const EventGroup &a, const EventGroup &b) {
			double scoreA = a.PurgeScoreEvent(sims, prob, pid);
			double scoreB = b.PurgeScoreEvent(sims, prob, pid);
			return scoreA > scoreB;
		});

		if (egs.size() > MAX_PLAYER_EVENT_GROUPS) {
			egs.resize(MAX_PLAYER_EVENT_GROUPS);
		}
		while (egs.size()>75 && egs.back().PurgeScoreEvent(sims, prob, pid) < MINEVENTSCORE*2.0) {
			egs.pop_back();
		}
		while (egs.size()>5 && egs.back().PurgeScoreEvent(sims, prob, pid) < MINEVENTSCORE) {
			egs.pop_back();
		}
		while (egs.size() && egs.back().PurgeScoreEvent(sims, prob, pid) < MINEVENTSCORE*0.5) {
			egs.pop_back();
		}

		if(prob>MINCHECKEVENTS) {
			//cerr << players[pid].name<<" has "<<egs.size()<<" event groups after removing irrelevants\n";
		}
		for (auto &eg : egs) {
			player_evgs.Add(eg);
		}
	}

	uint PlayerProcScore(uint pid, uint times_top, uint sims)
	{
		if (players[pid].player_id == 0) return 0;
		//count how many eventgroups they already have, from add important events...
		double overallprob = (double)times_top / (double)(sims);
		if (overallprob < MINCHECKEVENTS || times_top>=sims-0)
		{
			return 3;
		}
		return 130-(uint)(abs(overallprob-0.5)*10.0);
	}

	void ProcessPlayerEventsInitial(uint pid, uint times_top, vector<bool> &made_tops, vector<Simulation> &sims, RayRand64 &rng)
	{
		double overallprob = (double)times_top / (double)(sims.size());
		if (overallprob < MINCHECKEVENTS || times_top>=sims.size()-0)
		{
			CountEventGroups(sims, made_tops, event_groups, pid DEBUGARGS);
			RemoveIrrelevantEventGroups((uint)sims.size(), times_top, pid, event_groups DEBUGARGS);
			return;
		}

		//count events
		PlayerEvents events;
		auto countevents_time = GetMilliCount();
		CountEventsPlayer(times_top, made_tops, sims, events, pid DEBUGARGS);
		countevents_time = GetMilliSpan(countevents_time);

		//remove irrelevant events
		auto removeirrelevant_time = GetMilliCount();
		RemoveIrrelevantEvents((uint)sims.size(), times_top, events, pid DEBUGARGS);
		removeirrelevant_time = GetMilliSpan(removeirrelevant_time);

		//group events, only find for multiple events here
		//the set of single events that are used to make the grouped events should automatically be added to the eventgroups
			//then I can reduce the resolution on the grouping of the events and not even have it check for single events
		auto groupevents_time = GetMilliCount();
		GroupEvents(sims, times_top, made_tops, events, pid, event_groups, rng DEBUGARGS);
		groupevents_time = GetMilliSpan(groupevents_time);

		//count event groups prelim
		auto counteventgroupsprelim_time = GetMilliCount();
		CountEventGroupsPrelim(sims, made_tops, event_groups, pid DEBUGARGS);
		counteventgroupsprelim_time = GetMilliSpan(counteventgroupsprelim_time);
		
		//remove irrelevant event groups prelim
		auto removeirrelevantgroupsprelim_time = GetMilliCount();
		RemoveIrrelevantEventGroupsPrelim((uint)sims.size(), times_top, pid, event_groups DEBUGARGS);
		removeirrelevantgroupsprelim_time = GetMilliSpan(removeirrelevantgroupsprelim_time);
		
		//count event groups
		auto counteventgroups_time = GetMilliCount();
		CountEventGroups(sims, made_tops, event_groups, pid DEBUGARGS);
		counteventgroups_time = GetMilliSpan(counteventgroups_time);

		//remove irrelevant event groups
		auto removeirrelevantgroups_time = GetMilliCount();
		RemoveIrrelevantEventGroups((uint)sims.size(), times_top, pid, event_groups DEBUGARGS);
		removeirrelevantgroups_time = GetMilliSpan(removeirrelevantgroups_time);

		//event groups shrink to fit
		//cerr << players[pid].player_id <<": "<< players[pid].name << " times = " << countevents_time << ", " << removeirrelevant_time << ", " << groupevents_time << ", "<<counteventgroupsprelim_time<<", "<<removeirrelevantgroupsprelim_time<<", " << counteventgroups_time << ", " << removeirrelevantgroups_time << "\n";
		//cerr << players[pid].player_id <<": "<< players[pid].name << " times = " << countevents_time << ", " << groupevents_time << ", "<<counteventgroupsprelim_time<<", " << counteventgroups_time << "\n";
		event_groups.shrink_to_fit(pid);
	}

	void PlayersInitialEvents(uint start_pid, uint end_pid, vector<Simulation> &sims, array<uint,MAX_PLAYER_ID> &players_times_top, /*vector< pair<uint,uint> > &players_times_top,*/ vector<vector<bool> > &made_tops)
	{
		uint proced=0;
		//cerr << "I got pids "<<start_pid<<" to "<<end_pid<<"\n";
		RayRand64 rng;
		rng.SetStrongSeed((unsigned __int64)time(0)+(unsigned __int64)start_pid+(unsigned __int64)GetMilliCount());
		for(uint i=start_pid;i<=end_pid;i++) {
			if(players[i].player_id==0) continue;
			ProcessPlayerEventsInitial(i, players_times_top[i], made_tops[i], sims, rng);
			proced++;
		}
		//cerr << "processed "<<proced<<" players from "<<start_pid<<" to "<<end_pid<<", over "<<sims.size()<<" samples, proced*samples=="<<(proced*(uint)sims.size())<<"\n";
	}

	void ProcessPlayerEvents(uint pid, uint times_top, vector<bool> &made_tops, vector<Simulation> &sims, RayRand64 &rng)
	{
		CountEventGroups(sims, made_tops, event_groups, pid DEBUGARGS);
	}

	void PlayersProcessEvents(uint start_pid, uint end_pid, vector<Simulation> &sims, array<uint,MAX_PLAYER_ID> &players_times_top, /*vector< pair<uint,uint> > &players_times_top,*/ vector<vector<bool> > &made_tops)
	{
		uint proced=0;
		//cerr << "I got pids "<<start_pid<<" to "<<end_pid<<"\n";
		RayRand64 rng;
		rng.SetStrongSeed((unsigned __int64)time(0)+(unsigned __int64)start_pid+(unsigned __int64)GetMilliCount());
		for(uint i=start_pid;i<=end_pid;i++) {
			if(players[i].player_id==0) continue;
			ProcessPlayerEvents(i, players_times_top[i], made_tops[i], sims, rng);
			proced++;
		}
		//cerr << "processed "<<proced<<" players from "<<start_pid<<" to "<<end_pid<<", over "<<sims.size()<<" samples, proced*samples=="<<(proced*(uint)sims.size())<<"\n";
	}

	void ClearCounters()
	{
		for (auto &t : players_times_top) t = 0;
		for (uint pid = 0; pid < MAX_PLAYER_ID; pid++) {
			auto &evgs = event_groups.playereventgroups[pid];
			size_t total = 0;
			for (size_t i = 0; i < EVGS_HASH_SIZE; i++) {
				total += evgs.eventgroups[i].size();
				for (auto &e : evgs.eventgroups[i]) {
					e.hits = 0;
					e.misses = 0;
				}
			}
		}
		for (auto &m : mods) {
			m->ClearCounters();
		}
	}

	void run_initial_block(vector<Simulation> &origsims, atomic<int> &threads DEBUGARGSDEF)
	{
		threads++;//start CPU intensive work
		vector<Simulation> tsims;
		vector<Simulation> *psims;
		auto filtertime=GetMilliCount();
		if (typeid(*this) == typeid(SimBranch)) {
			psims = &origsims;
		}
		else {
			Filter(origsims, tsims);
			psims = &tsims;
			filtertime = GetMilliSpan(filtertime);
			cerr << tsims.size() << " samples found (" << branch_desc << ")! filtering took " << filtertime << "ms\n";
		}
		vector<Simulation> &sims = *psims;
		last_samples = (uint)sims.size();
		ClearCounters();
		if (sims.size() == 0) {
			threads--;
			return;
		}

		atomic<int> my_threads(0);

		//PurgeInsignificantEvents(sims DEBUGARGS);
		//add important events, should this add important events from all sims, if a branch is low res enough it might not find a tournament win event in the initial block...
		AddImportantEvents(sims);
		
		//vector< pair<uint,uint> > players_times_top(MAX_PLAYER_ID);
		for (auto &t : players_times_top) t = 0;
		vector<vector<bool> > made_tops(MAX_PLAYER_ID);
		for(uint i=0;i<sims.size();i++) {
			auto &s=sims[i];
			//for(auto &p : s.player_points) {
			for (uint a = 0; a < TOP_NUM_QUALIFY;a++) {
				auto &p = s.player_points[a];
				//players_times_top[p.first].first=p.first;
				players_times_top[p.first]++;
				if (made_tops[p.first].size() == 0) made_tops[p.first].resize(sims.size());
				made_tops[p.first][i]=1;
			}
		}

		for(auto &m : mods) {
			threads++;
			my_threads++;
			thread([&threads, &my_threads, &m, &sims, this](){
				m->Proc(sims, players_times_top);
				threads--;
				my_threads--;
			}).detach();
			//while (BDEBUG && threads > 0) RaySleep(1000 * threads);
		}
		
		//figure out how to split threads?
		uint total_proc_score=0;
		for (uint i = 0; i < players_times_top.size();i++) {
			auto &p = players_times_top[i];
			if (players[i].player_id == 0) continue;
			//if(p.first==0) continue;
			total_proc_score += PlayerProcScore(i, p, (uint)sims.size());
		}
		uint MY_THREADS=THREADS;
		MY_THREADS=MY_THREADS*(uint)sims.size()*THREADS/BLOCK_SIZE +1;
		MY_THREADS=std::max<uint>(THREADS/2,MY_THREADS);
		//if(sims.size()==BLOCK_SIZE) MY_THREADS*=THREADS;
		threads--;//no more big CPU intensive stuff on this main thread
		cerr << "MY_THREADS=="<<MY_THREADS<<", sims.size()=="<<sims.size()<<"\n";

		uint proc_score_per_thread=total_proc_score/MY_THREADS;//the last thread gets the leftovers
		uint start_pid=0;
		uint tproc_score=0;
		for(uint p=0;MY_THREADS>1 && p<players_times_top.size();p++) {
			if(players[p].player_id==0) continue;
			tproc_score += PlayerProcScore(p, players_times_top[p], (uint)sims.size());
			if(tproc_score>=proc_score_per_thread) {
				tproc_score=0;
				while(threads>=THREADS) RaySleep(10000);
				threads++;
				my_threads++;
				thread([&threads, &my_threads, start_pid, p, &sims, &made_tops, this](){
					PlayersInitialEvents(start_pid, p, sims, players_times_top, made_tops);
					threads--;
					my_threads--;
				}).detach();
				start_pid=p+1;
			}
		}
		if(MY_THREADS==1 || (tproc_score>0 && start_pid<players_times_top.size()) ) {
			PlayersInitialEvents(start_pid, (uint)players_times_top.size()-1, sims, players_times_top, made_tops);
		}

		while(my_threads>0) RaySleep(10000*my_threads);
		//save
		cerr << "done processing events\n";
		DisplayStats((uint)sims.size(), players_times_top);
	}

	void run_block(vector<Simulation> &origsims, atomic<int> &threads DEBUGARGSDEF)
	{
		threads++;//start CPU intensive work
		vector<Simulation> tsims;
		vector<Simulation> *psims;
		auto filtertime = GetMilliCount();
		if (typeid(*this) == typeid(SimBranch)) {
			psims = &origsims;
		}
		else {
			Filter(origsims, tsims);
			psims = &tsims;
			filtertime = GetMilliSpan(filtertime);
			cerr << tsims.size() << " samples found (" << branch_desc << ")! filtering took " << filtertime << "ms\n";
		}
		vector<Simulation> &sims = *psims;
		last_samples = (uint)sims.size();
		ClearCounters();
		if (sims.size() == 0) {
			threads--;
			return;
		}
		atomic<int> my_threads(0);
		
		//vector< pair<uint,uint> > players_times_top(MAX_PLAYER_ID);
		for (auto &t : players_times_top) t = 0;
		vector<vector<bool> > made_tops(MAX_PLAYER_ID);
		for(uint i=0;i<sims.size();i++) {
			auto &s=sims[i];
			//for(auto &p : s.player_points) {
			for (uint a = 0; a < TOP_NUM_QUALIFY; a++) {
				auto &p = s.player_points[a];
				//players_times_top[p.first].first=p.first;
				players_times_top[p.first]++;
				if (made_tops[p.first].size() == 0) made_tops[p.first].resize(sims.size());
				made_tops[p.first][i]=1;
			}
		}

		for(auto &m : mods) {
			threads++;
			my_threads++;
			thread([&threads,&my_threads,&m,&sims,this](){
				m->Proc(sims, players_times_top);
				threads--;
				my_threads--;
			}).detach();
			//while (BDEBUG && threads > 0) RaySleep(1000 * threads);
		}

		//figure out how to split threads?
		uint total_proc_score=0;
		for (uint i = 0; i < players_times_top.size();i++) {
			auto p = players_times_top[i];
			if(players[i].player_id==0) continue;
			total_proc_score += PlayerProcScore(i, p, (uint)sims.size());
		}
		uint MY_THREADS=THREADS;
		MY_THREADS=MY_THREADS*(uint)sims.size()*THREADS/BLOCK_SIZE +1;
		MY_THREADS=std::max<uint>(THREADS/2,MY_THREADS);
		//if(sims.size()==BLOCK_SIZE) MY_THREADS*=THREADS;
		threads--;//no more big CPU intensive stuff on this main thread
		//cerr << "MY_THREADS=="<<MY_THREADS<<", sims.size()=="<<sims.size()<<"\n";

		uint proc_score_per_thread=total_proc_score/MY_THREADS;//the last thread gets the leftovers
		uint start_pid=0;
		uint tproc_score=0;
		for(uint p=0;MY_THREADS>1 && p<players_times_top.size();p++) {
			if(players[p].player_id==0) continue;
			tproc_score += PlayerProcScore(p, players_times_top[p], (uint)sims.size());
			if(tproc_score>=proc_score_per_thread) {
				tproc_score=0;
				while(threads>=THREADS) RaySleep(10000);
				threads++;
				my_threads++;
				thread([&threads, &my_threads, start_pid, p, &sims,&made_tops,this](){
					PlayersProcessEvents(start_pid, p, sims, players_times_top, made_tops);
					threads--;
					my_threads--;
				}).detach();
				start_pid=p+1;
			}
		}
		if(MY_THREADS==1 || (tproc_score>0 && start_pid<players_times_top.size()) ) {
			PlayersProcessEvents(start_pid, (uint)players_times_top.size()-1, sims, players_times_top, made_tops);
		}

		while(my_threads>0) RaySleep(10000*my_threads);
		cerr << "done processing events\n";
		//save
	}
};

class BranchTournamentPlacing : public SimBranch
{
protected:
	uint pid;
	uint tid;
	uint placing;
	Event ev;
	
	bool _MatchSim(Simulation &sim)
	{
		if(sim.FindEvent(ev)) return true;
		return false;
	}
	
public:
	BranchTournamentPlacing(uint PID, uint TID, uint Placing, string Description) : SimBranch(Description)
	{
		pid=PID;
		tid=TID;
		placing=Placing;
		for (auto &u : upcoming_matches) {
			if (u.t_id != tid) continue;
			if (u.round != placing) continue;
			for (auto p : u.players) {
				if (p == pid) {
					tid = u.id;
					placing = UPCOMING_LOSE;
				}
			}
		}
		ev=Event(tid, pid, placing);
		type = 2;
		//branch_desc = players[PID].name.ToString() + " gets "+ToString(Placing)+" place in " + TournamentName;
		//branch_desc = Description;
	}
	
	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}
	
	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for(auto &s : origsims) if(_MatchSim(s)) sims.push_back(s);
	}
};

class BranchTournamentExists : public SimBranch
{
protected:
	uint tid;
	bool exists;

	bool _MatchSim(Simulation &sim)
	{
		for (auto &ev : sim.events) if (ev.t_id == tid) return exists;
		return (!exists);
	}

public:
	BranchTournamentExists(uint TID, bool Exists, string Description) : SimBranch(Description)
	{
		tid = TID;
		exists = Exists;
		type = 5;
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

class BranchEventList : public SimBranch
{
protected:
	bool _MatchSim(Simulation &sim)
	{
		for (auto &e : match_events) {
			bool good = false;
			for (uint i = 0; i < e.size(); i++) {
				if (sim.FindEvent(e[i])) good = true;
			}
			if (!good) return false;
		}
		return true;
	}
public:
	BranchEventList(string Description) : SimBranch(Description)
	{
		type = 3;
	}

	vector<vector<Event>> match_events;

	void AddEvent(uint TID, uint PID, uint Placing)
	{
		Event e(TID, PID, Placing);
		vector<Event> v;
		v.push_back(e);
		match_events.push_back(v);
	}

	void AddOrEvents(vector<Event> &events)
	{
		match_events.push_back(events);
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

class BranchOrEventList : public SimBranch
{
protected:
	bool _MatchSim(Simulation &sim)
	{
		for (auto &e : match_events) {
			uint count = 0;
			for (uint i = 0; i < e.size(); i++) {
				if (sim.FindEvent(e[i])) count++;
			}
			if (count == e.size()) return true;
		}
		return false;
	}
public:
	BranchOrEventList(string Description) : SimBranch(Description)
	{
		type = 6;
	}

	vector<vector<Event>> match_events;

	void AddEvent(uint TID, uint PID, uint Placing)
	{
		Event e(TID, PID, Placing);
		vector<Event> v;
		v.push_back(e);
		match_events.push_back(v);
	}

	void AddAndEvents(vector<Event> &events)
	{
		match_events.push_back(events);
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

class BranchChampionships : public SimBranch
{
protected:
	bool _MatchSim(Simulation &sim)
	{
		uint c = 0;
		for (auto &e : sim.events) {
			if (e.p_id == pid && e.placing == 1) c++;
		}
		if (op == 0 && c == num) return true;
		if (op == 1 && c > num) return true;
		if (op == -1 && c < num) return true;
		return false;
	}
public:
	uint pid;
	uint num;
	int op;
	BranchChampionships(uint PID, uint NUM, string Operator, string Description) : SimBranch(Description)
	{
		type = 7;
		pid = PID;
		num = NUM;
		op = 0;
		if (Operator == "==") op = 0;
		else if (Operator == ">") op = 1;
		else if (Operator == "<") op = -1;
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

class BranchBlizzconQualification : public SimBranch
{
protected:
	uint pid;
	bool qualify;

	bool _MatchSim(Simulation &sim)
	{
		if (sim.IsQualified(pid)==qualify) return true;
		return false;
	}

public:
	BranchBlizzconQualification(uint PID, bool Qualify, string Description) : SimBranch(Description)
	{
		pid = PID;
		qualify = Qualify;
		type = 4;
	}

	BranchBlizzconQualification(BranchBlizzconQualification &&o) : SimBranch(std::move(o))
	{
		pid = o.pid;
		qualify = o.qualify;
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

class BranchDoubleBlizzconQualification : public SimBranch
{
protected:
	uint pid1;
	uint pid2;
	bool qualify;

	bool _MatchSim(Simulation &sim)
	{
		if (sim.IsQualified(pid1) != qualify) return false;
		if (sim.IsQualified(pid2) != qualify) return false;
		return true;
	}

public:
	BranchDoubleBlizzconQualification(uint PID1, uint PID2, bool Qualify, string Description) : SimBranch(Description)
	{
		pid1 = PID1;
		pid2 = PID2;
		qualify = Qualify;
		type = 4;
	}

	BranchDoubleBlizzconQualification(BranchDoubleBlizzconQualification &&o) : SimBranch(std::move(o))
	{
		pid1 = o.pid1;
		pid2 = o.pid2;
		qualify = o.qualify;
	}

	virtual bool MatchSim(Simulation &sim) {
		return _MatchSim(sim);
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (_MatchSim(s)) sims.push_back(s);
	}
};

template<class B>
class BranchVerbose : public B
{
public:
	BranchVerbose(B &&base) : B(std::move(base))
	{
		//type = 3;
	}

	virtual bool MatchSim(Simulation &sim) {
		//return _MatchSim(sim);
		if (B::_MatchSim(sim)) {
			OutputEvents(sim);
			return true;
		}
		return false;
	}

	virtual void Filter(vector<Simulation> &origsims, vector<Simulation> &sims)
	{
		for (auto &s : origsims) if (B::_MatchSim(s)) sims.push_back(s);
		/*for (auto &s : sims) {
			OutputEvents(s);
		}*/
	}

	void OutputEvents(Simulation &s)
	{
		string out = s.Output();
		//cerr << out;
		DEBUGOUT(FATAL, B::branch_desc, out);
	}
};

class Simulator
{
public:
	//WCS2015 wcs;
	/*void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//wcs.predict(sim, rng DEBUGARGS);
	}*/

	void EnableGodModesNewbModes()
	{
	}

	void predict(vector<Simulation> &sims, vector<unique_ptr<SimBranch> > &branches, array<vector<PointsCounter>,THREADS> &pointscounters, uint block DEBUGARGSDEF)
	{
		auto sp = GetMilliCount();
		cerr << "starting predictions\n";
		atomic<int> threads(0);
		
		for (uint t = 0; t < THREADS; t++) {
			size_t start = BLOCK_SIZE / THREADS * t;
			size_t end = start + BLOCK_SIZE / THREADS;

			if (t == THREADS - 1)
				end = sims.size();
			if (t == 0)
				assert(start == 0);
			threads++;
			auto &pointscounter=pointscounters[t];
			thread([&threads, &sims, &pointscounter, &branches, start, end, block, this]() {
				auto self = wcs;
				RayRand64 rng;
				rng.SetStrongSeed((unsigned __int64)time(0)+(unsigned __int64)start+(unsigned __int64)GetMilliCount());
				if (BENCHMARK) rng.SetStrongSeed(start + 12321 + block * 117);//don't use this for fanfics cause aligulac ratings or any other change ruins it anyways
				//cerr << "starting thread " << start << "..." << end << "\n";
				uint last_events_size = 50;
				uint last_player_points_size = 100;
				const size_t step = 4;
				for (size_t i = start; i < end; i+=step) {
					size_t s_end = min(end, i + step);
					for (size_t a = i; a < s_end; a++) {
						Simulation &sim = sims[a];
						sim.events.reserve(last_events_size);
						sim.player_points.reserve(last_player_points_size);

						self.predict(sim, rng DEBUGARGS);
						sim.SortPlayers(rng);
						for (uint i = 0; i < TOP_NUM_QUALIFY; i++) {
							sim.AddEvent(1, sim.player_points[i].first, QUALIFY_BLIZZCON);//I wanted to put the seed as the t_id, but that screws up the grouping
						}
						last_events_size = max((uint)sim.events.capacity(), last_events_size);
						last_player_points_size = max((uint)sim.player_points.capacity(), last_player_points_size);

						sim.SortEvents();//need this to match events
						//sim.RecalcBloomFilter();
					}
					
					for (uint b = 0; b < branches.size(); b++) {
						for (size_t a = i; a < s_end; a++) {
							Simulation &sim = sims[a];
							//if (b == 0) sim.SortPlayers(rng);
							if (branches[b]->MatchSim(sim)) {
								sim.CountPoints(pointscounter[b]);
							}
						}
					}
					for (size_t a = i; a < s_end; a++) {
						Simulation &sim = sims[a];
						sim.CutPlayers();
					}
					
					/*array<Player*,TOP_NUM_QUALIFY> seedings;
					for(uint i=0;i<TOP_NUM_QUALIFY;i++) {
						uint pid=sim.player_points[i].first;
						seedings[i]=&players[pid];
					}
					self.wcs.blizzcon.predict(sim, rng, seedings DEBUGARGS);*/
				}
				//cerr << "completed thread " << start << "..." << end << "\n";
				threads--;
			}).detach();
		}

		while (threads>0) RaySleep(10000*threads);
		cerr << "predicting took "<<GetMilliSpan(sp)<<" ms\n";

		if (sims[0].events.size() > sims[0].events.capacity()) {
			throw rayexception("WTF! too many events!" DEBUGARGS);
		}
	}

	void run_initial_block(vector<unique_ptr<SimBranch> > &branches DEBUGARGSDEF)
	{
		auto s = GetMilliCount();
		MysqlCon con;
		if (BENCHMARK==false && BDEBUG==0) {
			con.ConnectToMysql("../sc2_mysqlcreds.txt");
			con.StartTransaction();
		}
		cerr << "allocating simulations\n";
		vector<Simulation> sims(BLOCK_SIZE);
		array<vector<PointsCounter>,THREADS> pointscounters;
		for(auto &p : pointscounters) p.resize(branches.size());

		atomic<int> threads(0);
		atomic<int> branch_threads(0);
		threads++;
		thread([this, &threads, &sims, &branches, &pointscounters](){
			predict(sims, branches, pointscounters, 0 DEBUGARGS);
			threads--;
		}).detach();
		//while (BDEBUG && threads>0) RaySleep(10000 * threads);

		size_t base_sim_id = 0;
		for (uint i = 0; i < branches.size(); i++) {
			if (BENCHMARK || BDEBUG) break;
			//if (BDEBUG) break;
			auto &base = *branches[0];
			auto &b = *branches[i];
			if (i == 0 && b.type==1) {
				b.init_db_base(con, base_sim_id DEBUGARGS);
				base_sim_id = b.sim_id;
			}
			else b.init_db(con, base_sim_id DEBUGARGS);
		}
		while (threads>0) RaySleep(100000);
		for(uint i=0;i<branches.size();i++) {
			auto &b=*branches[i];
			while (threads>=THREADS || branch_threads>=THREADS) RaySleep(30000);//only allow some branches at a time, side branches are pretty quick anyways
			//threads++;
			branch_threads++;
			//if (i == 0 && b.type==1) threads += THREADS;
			thread([&threads, &branch_threads, &sims, &b, &pointscounters, i](){
				for (uint t = 0; t<THREADS; t++) {
					b.ConsumePointsCounter(pointscounters[t][i]);
				}
				b.run_initial_block(sims, threads DEBUGARGS);
				//threads--;
				branch_threads--;
			}).detach();
			//while (BDEBUG && threads>0) RaySleep(10000 * threads);
			RaySleep(100000);
			if (i == 0 && b.type == 1) {
				RaySleep(1000 * 1000 * 5);//while (threads > 0) RaySleep(10000);
			}
		}
		while (threads>0 || branch_threads>0) RaySleep(10000 * (threads+1));
		//cerr << "done with initial block, took " << GetMilliSpan(s) << " ms\n";
		DEBUGOUT(FATAL, GetMilliSpan(s), "inital block completion time");
		s = GetMilliCount();
		if (/*(!DEBUG) &&*/ (!BENCHMARK) && (!BDEBUG)) {
			for (uint i = 0; i < branches.size(); i++) {
				auto &b = *branches[i];
				b.init_save(con DEBUGARGS);
			}
			con.Commit();
		}
		cerr << "done writing to database, took " << GetMilliSpan(s) << " ms\n";
	}

	void run_block(vector<unique_ptr<SimBranch> > &branches, uint block DEBUGARGSDEF)
	{
		auto s = GetMilliCount();
		cerr << "allocating simulations\n";
		vector<Simulation> sims(BLOCK_SIZE);
		array<vector<PointsCounter>,THREADS> pointscounters;
		for(auto &p : pointscounters) p.resize(branches.size());
		predict(sims, branches, pointscounters, block DEBUGARGS);

		atomic<int> threads(0);
		atomic<int> branch_threads(0);
		for(uint i=0;i<branches.size();i++) {
			auto &b=*branches[i];
			while (threads >= THREADS || branch_threads >= THREADS) RaySleep(30000);//only allow some branches at a time, side branches are pretty quick anyways
			//threads++;
			branch_threads++;
			//if (i == 0 && b.type==1) threads += THREADS;
			thread([&threads, &branch_threads, &sims, &b, &pointscounters, i](){
				for(uint t=0;t<THREADS;t++) {
					b.ConsumePointsCounter(pointscounters[t][i]);
				}
				b.run_block(sims, threads DEBUGARGS);

				auto ds = GetMilliCount();
				while (/*(!BDEBUG) &&*/ (!BENCHMARK) && (!BDEBUG)) {
					try {
						MysqlCon con;
						con.ConnectToMysql("../sc2_mysqlcreds.txt");
						con.StartTransaction();
						b.save(con DEBUGARGS);
						con.Commit();
						break;
					}
					catch (...) {
						cerr << "Trying DB again...\n";
						RaySleep(1000 * 1000 * 5);
					}
				}
				//cerr << "db for branch took " << GetMilliSpan(ds) << "ms\n";
				//threads--;
				branch_threads--;
				//if (i == 0 && b.type==1) threads -= THREADS;
			}).detach();
			//while (BDEBUG && threads>0) RaySleep(10000 * threads);
			RaySleep(100000);
			if (i == 0 && b.type == 1) {
				RaySleep(1000 * 1000 * 5);// while (threads > 0) RaySleep(10000);
			}
		}
		while (threads>0 || branch_threads>0) RaySleep(10000 * (threads+1));
		//cerr << "done with block, took " << GetMilliSpan(s) << " ms\n";
		DEBUGOUT(FATAL, GetMilliSpan(s), "block completion time");
		/*s = GetMilliCount();
		while (!BENCHMARK) {
			try {
				MysqlCon con;
				con.ConnectToMysql("../sc2_mysqlcreds.txt");
				con.StartTransaction();
				for (uint i = 0; i < branches.size(); i++) {
					auto &b = *branches[i];
					b.save(con DEBUGARGS);
				}
				con.Commit();
				break;
			}
			catch (...) {
				cerr << "Trying DB again...\n";
				RaySleep(1000 * 1000 * 5);
			}
		}
		cerr << "done writing to database, took " << GetMilliSpan(s) << " ms\n";*/
	}

	void DisplayStats(EventGroupsPlayers &event_groups DEBUGARGSDEF)
	{
	}

	void run(DEBUGARGDEF)
	{
		LoadPlayers(DEBUGARG);
		init(DEBUGARG);

		EnableGodModesNewbModes();

		PointsCounter points_counter;
		EventGroupsPlayers event_groups;

		vector<unique_ptr<SimBranch> > branches;
		branches.push_back( unique_ptr<SimBranch>(new SimBranch(string())) );//main branch

		/*auto lilbowbeatsforgg = new BranchEventList("Lilbow beats ForGG in the WCS finals");
		lilbowbeatsforgg->AddEvent(30, 5064, 1);
		lilbowbeatsforgg->AddEvent(30, 34, 2);
		branches.push_back(unique_ptr<BranchEventList>(lilbowbeatsforgg));
		auto forggbeatslilbow = new BranchEventList("ForGG beats Lilbow in the WCS finals");
		forggbeatslilbow->AddEvent(30, 34, 1);
		forggbeatslilbow->AddEvent(30, 5064, 2);
		branches.push_back(unique_ptr<BranchEventList>(forggbeatslilbow));*/

		/*branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("jin air;sos;p;kr" DEBUGARGS).player_id, 48, 1, "sOs wins DreamHack Stockholm"))
			);
		branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("dpix;fantasy;t;kr" DEBUGARGS).player_id, 48, 1, "Fantasy wins DreamHack Stockholm"))
			);
		branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("roccat;hyun;z;kr" DEBUGARGS).player_id, 48, 1, "Hyun wins DreamHack Stockholm"))
			);
		branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("skt t1;dark;z;kr" DEBUGARGS).player_id, 48, 1, "Dark wins DreamHack Stockholm"))
			);
		branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("millenium;forgg;t;kr" DEBUGARGS).player_id, 48, 1, "ForGG wins DreamHack Stockholm"))
			);*/
		/*vector<uint> dreamhack_pids = { SearchPlayer("jin air;sos;p;kr" DEBUGARGS).player_id, SearchPlayer("roccat;hyun;z;kr" DEBUGARGS).player_id, SearchPlayer("samsung;solar;z;kr" DEBUGARGS).player_id };
		for (auto p : dreamhack_pids) {
			branches.push_back(unique_ptr<BranchTournamentPlacing>(
				new BranchTournamentPlacing(p, 48, 1, players[p].name.ToString()+" wins DreamHack Stockholm"))
				);
			branches.push_back(unique_ptr<BranchTournamentPlacing>(
				new BranchTournamentPlacing(p, 48, 2, players[p].name.ToString() + " gets 2nd in DreamHack Stockholm"))
				);
		}*/

		//add a branch for every upcoming match! that's madness! THIS! IS! WCS PREDICTOR!
		for (auto &u : upcoming_matches) {
			uint num_players = 0;
			if (u.round > 48) continue;
			for (auto pid : u.players) {
				if (pid == 0) continue;
				num_players++;
			}
			for (auto pid : u.players) {
				if (pid == 0) continue;
				auto &p = players[pid];
				auto uwb = new BranchEventList(p.name.ToString() + " wins their upcoming match in the " + TournamentNames[u.t_id] + " round of " + ToString(u.round));
				uwb->AddEvent(u.id, pid, UPCOMING_WIN);
				branches.push_back(unique_ptr<BranchEventList>(uwb));

				if (num_players > 2 && u.round<=32) {//this might be too extreme, especially when Dreamhack announces groups
					auto ulb = new BranchEventList(p.name.ToString() + " loses their upcoming match in the " + TournamentNames[u.t_id] + " round of " + ToString(u.round));
					ulb->AddEvent(u.id, pid, UPCOMING_LOSE);
					branches.push_back(unique_ptr<BranchEventList>(ulb));
				}
			}
		}

		/*branches.push_back(unique_ptr<BranchBlizzconQualification>(
			new BranchBlizzconQualification(SearchPlayer("millenium;lilbow;p;fr" DEBUGARGS).player_id, false, "Lilbow doesn't qualify for Blizzcon"))
			);*/
		/*branches.push_back(unique_ptr<BranchBlizzconQualification>(
			new BranchBlizzconQualification(SearchPlayer("samsung;dear;p;kr" DEBUGARGS).player_id, true, "Dear qualifies for Blizzcon"))
			);*/
		/*branches.clear();
		branches.push_back(unique_ptr<BranchVerbose<BranchBlizzconQualification>>(
			new BranchVerbose<BranchBlizzconQualification>(BranchBlizzconQualification(SearchPlayer("millenium;lilbow;p;fr" DEBUGARGS).player_id, false, "Lilbow doesn't qualify for Blizzcon")))
			);*/

		/*branches.push_back(unique_ptr<BranchBlizzconQualification>(
			new BranchBlizzconQualification(SearchPlayer("millenium;forgg;t;kr" DEBUGARGS).player_id, true, "ForGG qualifies for Blizzcon"))
			);
		branches.push_back(unique_ptr<BranchBlizzconQualification>(
			new BranchBlizzconQualification(SearchPlayer("millenium;lilbow;p;fr" DEBUGARGS).player_id, true, "Lilbow qualifies for Blizzcon"))
			);
		branches.push_back(unique_ptr<BranchDoubleBlizzconQualification>(
			new BranchDoubleBlizzconQualification(SearchPlayer("millenium;lilbow;p;fr" DEBUGARGS).player_id, SearchPlayer("millenium;forgg;t;kr" DEBUGARGS).player_id, true, "Lilbow and ForGG qualify for Blizzcon"))
			);*/

		/*branches.push_back(unique_ptr<BranchChampionships>(
			new BranchChampionships(SearchPlayer("kt;flash;t;kr" DEBUGARGS).player_id, 1, ">", "Flash wins at least 2 tournaments"))
			);*/

		/*branches.push_back(unique_ptr<BranchTournamentExists>(
			new BranchTournamentExists((uint)wcs.placeholders.placeholders.size()+100, false, "No placeholder tournaments"))
			);*/

		/*
		branches.push_back(unique_ptr<BranchBlizzconQualification>(
		new BranchBlizzconQualification(SearchPlayer("cm storm;polt;t;kr" DEBUGARGS).player_id, false, "Polt doesn't qualify for Blizzcon"))
		);

		auto oddlyspecific = new BranchEventList("INnoVation wins SSL S3, ForGG wins WCS S3, soO wins GSL S3");
		//oddlyspecific->AddEvent(tid, pid, placing);
		oddlyspecific->AddEvent(32, 48, 1);
		oddlyspecific->AddEvent(30, 34, 1);
		oddlyspecific->AddEvent(27, 125, 1);
		branches.push_back(unique_ptr<BranchEventList>(oddlyspecific));*/
		/*branches.push_back(unique_ptr<BranchTournamentPlacing>(
			new BranchTournamentPlacing(SearchPlayer("dpix;fantasy;t;kr" DEBUGARGS).player_id, 58, 1, "FanTaSy wins KeSPA Cup 2"))
			);*/
			
		/*auto snute = new BranchEventList("Snute top 16 at DH Tours");
		snute->AddOrEvents(vector<Event>{ Event(46, 111, 16), Event(46, 111, 8), Event(46, 111, 4), Event(46, 111, 2), Event(46, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList("Snute top 8 at DH Tours");
		snute->AddOrEvents(vector<Event>{ Event(46, 111, 8), Event(46, 111, 4), Event(46, 111, 2), Event(46, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList("Snute gets 16th at DH Tours, Jaedong 8th at DH Tours");
		snute->AddEvent(46, 111, 16);
		snute->AddEvent(46, 73, 8);
		branches.push_back(unique_ptr<BranchEventList>(snute));
		auto p = snute;

		snute = new BranchEventList(p->branch_desc + ", top 16 at WCS S2");
		snute->match_events = p->match_events;
		snute->AddOrEvents(vector<Event>{ Event(18, 111, 8), Event(18,111,4), Event(18,111,2), Event(18,111,1), Event(18,111,16) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList(p->branch_desc + ", top 8 at WCS S2");
		snute->match_events = p->match_events;
		snute->AddOrEvents(vector<Event>{ Event(18, 111, 8), Event(18, 111, 4), Event(18, 111, 2), Event(18, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList(p->branch_desc + ", 16th at WCS S2");
		snute->match_events = p->match_events;
		snute->AddEvent(18, 111, 16);
		branches.push_back(unique_ptr<BranchEventList>(snute));
		p = snute;

		snute = new BranchEventList(p->branch_desc + ", top 4 at DH Stockholm");
		snute->match_events = p->match_events;
		snute->AddOrEvents(vector<Event>{ Event(48, 111, 4), Event(48, 111, 2), Event(48, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList(p->branch_desc + ", 1st at DH Stockholm");
		snute->match_events = p->match_events;
		snute->AddEvent(48, 111, 1);
		branches.push_back(unique_ptr<BranchEventList>(snute));
		p = snute;

		snute = new BranchEventList(p->branch_desc + ", 16th at WCS S3");
		snute->match_events = p->match_events;
		snute->AddEvent(30, 111, 16);
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList(p->branch_desc + ", top 8 at WCS S3");
		snute->match_events = p->match_events;
		snute->AddOrEvents(vector<Event>{ Event(30, 111, 8), Event(30, 111, 4), Event(30, 111, 2), Event(30, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		snute = new BranchEventList(p->branch_desc + ", top 4 at WCS S3");
		snute->match_events = p->match_events;
		snute->AddOrEvents(vector<Event>{ Event(30, 111, 4), Event(30, 111, 2), Event(30, 111, 1) });
		branches.push_back(unique_ptr<BranchEventList>(snute));

		auto vsnute = new BranchEventListVerbose(p->branch_desc + ", 8th at WCS S3, Jaedong 16th at WCS S3");
		vsnute->match_events = p->match_events;
		vsnute->AddEvent(30, 111, 8);
		vsnute->AddEvent(30, 73, 16);
		branches.push_back(unique_ptr<BranchEventListVerbose>(vsnute));*/

		cerr << "\n"<< branches.size() << " branches!\n";
		auto s = GetMilliCount();
		cerr << "Running initial block\n";
		run_initial_block(branches DEBUGARGS);
		//branches[0]->DisplayStats(DEBUGARGS);//this can only be run after the initial block since each block clears the counts?
		for (size_t b = 0; b < NUM_BLOCKS; b++) {
			cerr << "\nRunning block " << b+1 << "\n";
			try {
				run_block(branches, (uint)b+1 DEBUGARGS);
			} catch (rayexception ex) {
				ex.saywhat(DEBUGARG);
				b--;
			}
		}
		cerr << "\nDone with all blocks\n\n";
		auto totaltime = GetMilliSpan(s);
		//cerr << "Took "<<t<<" ms\n";
		DEBUGOUT(FATAL, totaltime, "");
	}

	void init(DEBUGARGDEF)
	{
		//run through old results
		wcs.init(DEBUGARG);

		if((!BENCHMARK) && (!BDEBUG)) {
			MysqlCon con;
			con.ConnectToMysql("../sc2_mysqlcreds.txt");
			con.StartTransaction();
			SavePlayers(con);
			headband_proc.Save(con DEBUGARGS);//<-----------------------------------------
			con.Commit();
		}
		/*else {
			MysqlCon con;
			con.ConnectToMysql("../sc2_mysqlcreds.txt");
			con.StartTransaction();
			headband_proc.Save(con DEBUGARGS);
			con.Commit();
		}*/
		//player rating decay here?
		cerr << "Done initializing\n";
	}
};

int main(int argc, const char *argv[])
{
	DEBUGOUT(FATAL, "program launch", "");
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << "\n";
		if (strcmp(argv[i], "afkmode") == 0) AFK = true;
	}
	//RaySleep(1000 * 1000 * 30);
	//return 0;
	if (!AFK) {
		cerr << "\nenter description: ";
		std::getline(cin, description);
	}
	if (description.length())
		cerr << "\ndescription: " << description << "\n\n";
	else cerr << "\nno description\n\n";

	cout << "sim 2015!\n\n";
#ifdef WIN32
	if(!BDEBUG /*&& !BENCHMARK*/)
	{
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
	}
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
#endif

	cerr << "sizeof(Event)==" << sizeof(Event) << "\n";
	if(BDEBUG) cerr << "IN DEBUG MODE!\n";
	if(BENCHMARK) cerr << "IN BENCHMARK MODE!\n";
	if(USE_ALIGULAC) cerr << "USING ALIGULAC RATINGS!\n";
	else cerr << "NOT USING ALIGULAC RATINGS!\n";
	cerr << "\n\n\n";
	EventCounter evc;
	evc.hits=1000;
	evc.misses=0;
	evc.placing=2;
	evc.p_id=6;
	evc.t_id=7;
	EventGroup evg;
	evg.AddEvent(evc);
	evg.hits=1000;
	evg.misses=0;
	cerr << "evc.ScoreEvent() == "<<evc.ScoreEvent(10000, 0.65, 6)<<", evg.ScoreEvent() == "<<evg.ScoreEvent(10000, 0.65, 6)<<"\n\n";

	Simulator simulator;
	simulator.run(DEBUGARG);
	return 0;
}
