
#include "tournaments_base.h"

class MatchCountry
{
public:
	char c[2];
	MatchCountry(char C[2])
	{
		c[0] = C[0];
		c[1] = C[1];
	}

	bool operator==(const char *t)
	{
		return *((short*)c) == *((short*)t);
	}
};
class FreePlayers
{
public:
	vector<Player *> koreans;
	vector<Player *> northamericans;
	vector<Player *> chinese;
	vector<Player *> latinamericans;
	vector<Player *> seas;//oceania, sea
	vector<Player *> taiwanese;//tw, hk, macau
	vector<Player *> europeans;
	//vector<Player *> others;

	void init()
	{
		for (auto &p : players) {
			if (p.player_id == 0) continue;
			if (p.last_period<last_period - 6) continue;

			add(&p);
		}
	}

	vector<Player*>& GetVector(Player *p)
	{
		if (p->player_id == 34)//forgg
			return europeans;
		if (p->player_id == 185)//stardust
			return northamericans;
		if (p->player_id == 73)//jaedong
			return northamericans;
		if (p->player_id == 19)//polt
			return northamericans;
		if (p->player_id == 8)//violet
			return northamericans;
		if (p->player_id == 1557)//hydra
			return northamericans;
		if (p->player_id == 365)//kingkong
			return seas;
		if (p->player_id == 1888)//fenner
			return seas;

		MatchCountry c = p->country;
		//string c = p->sCountry();
		if (c == "us" || c == "ca") {
			return northamericans;
		}
		else if (c == "kr") {
			return koreans;
		}
		else if (c == "cn") {
			return chinese;
		}
		else if (c == "mx" || c == "ar" || c == "br" || c == "pe" || c == "cl") {
			return latinamericans;
		}
		else if (c == "au" || c == "nz" || c == "jp" || c == "sg" || c == "th" || c == "ph" || c == "in") {
			return seas;
		}
		else if (c == "tw" || c == "hk") {
			return taiwanese;
		}
		else {
			return europeans;
		}
	}

	void remove_vec(vector<Player*> &vec, size_t i)
	{
		assert(vec.size()>i);
		vec[i] = vec.back();
		vec.pop_back();
	}

	void remove_vec(vector<Player*> &vec, Player *p)
	{
		/*if (p->player_id == 747) {
			cerr << p->name.ToString() << " remove\n";
		}*/
		for (uint i = 0; i<vec.size(); i++) {
			if (vec[i] == p) {
				remove_vec(vec, i);
				return;
			}
		}
		assert(0);
	}

	void try_remove_vec(vector<Player*> &vec, Player *p)
	{
		/*if (p->player_id == 747) {
			cerr << p->name.ToString() << " try remove\n";
		}*/
		for (uint i = 0; i<vec.size(); i++) {
			if (vec[i] == p) {
				remove_vec(vec, i);
				return;
			}
		}
	}

	void remove(Player *p)
	{
		remove_vec(GetVector(p), p);
	}

	void try_remove(Player *p)
	{
		try_remove_vec(GetVector(p), p);
	}

	void add_vec(vector<Player*> &vec, Player *p)
	{
		if (p->player_id == 0) return;
		/*if (p->player_id == 747) {
			cerr << p->name.ToString() << " add\n";
		}*/
		if (BDEBUG) {
			for (uint i = 0; i<vec.size(); i++) {
				assert(vec[i] != p);
			}
		}
		vec.push_back(p);
	}

	void add(Player *p)
	{
		add_vec(GetVector(p), p);
	}
};

class DreamhackBase : public TournamentBase
{
public:
	Round<32, SwissGroup, 8, RandomAdvancement<16, 16> > ro32;
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	DreamhackBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		//finals.match_placing_to_tournament_placing[1] = 2;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		finals.best_of = 5;
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		RandomAdvancement<32, 0> sortplayers;
		for (uint i = 0; i<32; i++) {
			Player *p = NULL;
			if (i<24) {
				p = good_players[rng.iRand64HQ() % good_players.size()];
			}
			else {
				//while(p==NULL || p->player_id==0) {
				p = &players[rng.iRand64HQ() % players.size()];
				//}
			}
			/*bool found=false;
			for(auto a : sortplayers.advancing_players) {
			if(a==p) {
			found=true;
			break;
			}
			}
			if(found) {*/
			if (player_map[p->player_id] == 1) {
				i--;
				continue;
			}
			player_map[p->player_id] = 1;
			sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		RandomAdvancement<16, 16> ro32adv;
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro32.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		ro32.PredictNoOldResults(sim, t_id, ro32adv, rng DEBUGARGS);
		ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16.PredictNoOldResults(sim, t_id, ro16adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.PredictNoOldResults(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.PredictNoOldResults(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.PredictNoOldResults(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class PlaceholderBase : public DreamhackBase
{
public:
	PlaceholderBase() : DreamhackBase("placeholder", 100)
	{
	}

	PlaceholderBase(const char *name, uint TID) : DreamhackBase(name, TID)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		DreamhackBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.ClearPlacings();
		ro16.ClearPlacings();
		quarterfinals.ClearPlacings();
		semifinals.ClearPlacings();
		//finals.match_placing_to_tournament_placing[1] = 0;
		//finals.ClearPlacings();
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//if(rng.dRand()>0.01) return;//1% chance to exist
		DreamhackBase::predict(sim, rng DEBUGARGS);
	}
};

/*class PlaceholderTournament1 : public PlaceholderBase
{
public:
PlaceholderTournament1() : PlaceholderBase("Placeholder Tournament 1",101)
{
}
};*/

class PlaceholderTournaments : public TournamentBase
{
public:
	//PlaceholderTournament1 p1;
	array<PlaceholderBase, 1> placeholders;

	PlaceholderTournaments() : TournamentBase("Placeholder Tournaments", 0)
	{
		for (uint i = 0; i<placeholders.size(); i++) {
			uint ttid = i + 101;
			placeholders[i].t_id = ttid;
			TournamentNames[ttid] = "Placeholder #" + ToString(i+1);
		}
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//p1.init(prev_matches, upcoming_matches DEBUGARGS);
		for (auto &p : placeholders) p.init(prev_matches, upcoming_matches DEBUGARGS);
		for (uint i = 0; i < placeholders.size()-1; i++) {
			placeholders[i].semifinals.ClearPlacings();
			placeholders[i].finals.ClearPlacings();
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		return;// :O no placeholders!
		if (rng.dRand()>0.1) {
			//sim.AddEvent(placeholders[0].t_id, 0, NO_PLACEHOLDERS);
			return;
		}
		//p1.predict(sim, rng DEBUGARGS);
		for (auto &p : placeholders) p.predict(sim, rng DEBUGARGS);
	}
};

class KRQualifiersBase : public TournamentBase
{
public:
	Round<96, SwissGroup, 24, StraightGroupAdvancement<48, 48> > ro96kr;
	Round<48, SwissGroup, 12, MixedAdvancement<24, 24> > ro48kr;

	KRQualifiersBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		ro96kr.ClearPlacings();
		ro48kr.ClearPlacings();
		ro96kr.best_of = 7;
		ro48kr.best_of = 7;
		ro48kr.match_placing_to_tournament_placing[0] = 1;
		ro48kr.match_placing_to_tournament_placing[1] = 1;
	}

	void predict(Simulation &sim, MixedAdvancement<24, 24> &QtoC, FreePlayers &frees, RayRand64 &rng DEBUGARGSDEF)
	{
		for (uint i = 0; i<96; i++) {
			size_t slot = 0;
			assert(frees.koreans.size()>0);
			slot = rng.iRand64HQ() % frees.koreans.size();
			for (uint c = 0; c<10 && i % 8 == 0 && frees.koreans[slot]->ratings[R]<1400; c++) {
				slot = rng.iRand64HQ() % frees.koreans.size();
			}
			ro96kr.matches[i / 4].players[i % 4] = frees.koreans[slot];
			frees.remove_vec(frees.koreans, slot);
		}

		StraightGroupAdvancement<48, 48> ro96toro48;
		ro96kr.PredictNoOldResults(sim, t_id, ro96toro48, rng DEBUGARGS);
		ro48kr.AcceptAdvancements(ro96toro48.advancing_players DEBUGARGS);
		for (auto p : ro96toro48.falling_players) {
			if (p) frees.koreans.push_back(p);
		}
		ro48kr.PredictNoOldResults(sim, t_id, QtoC, rng DEBUGARGS);
		for (auto p : QtoC.falling_players) {
			if (p) frees.koreans.push_back(p);
		}
	}
};

class CodeABase : public TournamentBase
{
public:
	Round<48, SingleMatch, 24, RandomAdvancement<24, 24>, true > matches;
	CodeABase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		matches.best_of = 5;
		matches.match_placing_to_tournament_placing[0] = 1;//24;
		matches.match_placing_to_tournament_placing[1] = 0;// 48;//24;
		//matches.points_for_placing[3] = matches.points_for_placing[2] = 50;
		matches.points_for_placing[1] = 50;
		//matches.ClearPlacings();
	}

	void init_s2(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		array<const char*, 48> ps = {
			"prime;yoda;t;kr", "SBENU;dongraegu;z;kr", "mvp;seed;p;kr", "skt t1;soo;z;kr", "liquid;taeja;t;kr", "skt t1;sorry;t;kr",
			"SBENU;hack;t;kr", "cj entus;bbyong;t;kr", "mvp;yonghwa;p;kr", "cj entus;byul;z;kr", "mvp;blaze;p;kr", "jin air;trap;p;kr",
			"axiom;heart;t;kr", "jin air;sos;p;kr", "kt;penguin;z;kr", "jin air;symbol;z;kr", ";first;p;kr", ";myungsik;p;kr",
			"samsung;dear;p;kr", "cj entus;ragnarok;z;kr", "SBENU;bomber;t;kr", "samsung;shine;z;kr", "yoe;san;p;kr", "myi;sacsri;z;kr",
			"skt t1;dream;t;kr", "root;supernova;t;kr", "jin air;maru;t;kr", "skt t1;billowy;p;kr", "myi;rain;p;kr", "jin air;cure;t;kr",
			"skt t1;dark;z;kr", "axiom;ryung;t;kr", "mvp;gumiho;t;kr", "skt t1;impact;z;kr", "kt;super;p;kr", "kt;flash;t;kr",
			"dpix;fantasy;t;kr", "jin air;pigbaby;p;kr", "jin air;terminator;p;kr", "liquid;hero;p;kr", "tcm;soulkey;z;kr", ";creator;p;kr",
			"mvp;marineking;t;kr", "dpix;patience;p;kr", "SBENU;curious;z;kr", "samsung;bravo;t;kr", ";mc;p;kr", "cj entus;trust;p;kr"
		};
		for (uint i = 0; i < ps.size(); i++) {
			auto p = &SearchPlayer(ps[i] DEBUGARGS);
			p->league = t_id;
			matches.matches[i / 2].players[i % 2] = p;
			frees.try_remove_vec(frees.koreans, p);
		}
		vector<string> urls;
		for (uint i = 0; i < 1; i++) {//section 3 has them all now
			urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_2/Code_A&action=edit&section=" + ToString(4 + i));
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint i = 0; i < ress.size(); i++) {
			vector<ScrapedGroup> smatches;
			ScrapeLiquipediaMatches(ress[i], t_id, smatches);
			for (auto &sm : smatches) {
				auto m = matches.GetMatch(sm.players);
				if (m) m->ReadOldResults(sm, prev_matches, upcoming_matches, matches.RoundOf());
				assert(m != NULL);
			}
		}
	}

	void init_s3(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		array<const char*, 48> ps = {
			";creator;p;kr", "jin air;pigbaby;p;kr", "samsung;solar;z;kr", "psistm;true;z;kr", "kt;ty;t;kr", "mvp;losira;z;kr",
			"skt t1;sorry;t;kr", "samsung;dear;p;kr", "prime;yoda;t;kr", "cj entus;skyhigh;t;kr", "cj entus;ragnarok;z;kr", "samsung;stork;p;kr",
			"liquid;hero;p;kr", "skt t1;classic;p;kr", "skt t1;soo;z;kr", "sbenu;leenock;z;kr", "acer;mma;t;kr", "mvp;yonghwa;p;kr",
			"mvp;blaze;p;kr", ";sleep;z;kr", "skt t1;innovation;t;kr", "prime;keen;t;kr", "skt t1;dark;z;kr", "sbenu;alive;t;kr",
			"cj entus;trust;p;kr", "sbenu;jjakji;t;kr", "cj entus;bbyong;t;kr", "kt;super;p;kr", "kt;life;z;kr", "jin air;trap;p;kr",
			"skt t1;dream;t;kr", "sbenu;hack;t;kr", "kt;flash;t;kr", "skt t1;impact;z;kr",
			"sbenu;bomber;t;kr", ";first;p;kr", "jin air;symbol;z;kr", "samsung;hurricane;p;kr", "myi;sacsri;z;kr", "jin air;cure;t;kr", "dpix;fantasy;t;kr", "yoe;san;p;kr",
			"dpix;patience;p;kr", "kt;zest;p;kr", "cj entus;hero;p;kr", "samsung;armani;z;kr", "mvp;gumiho;t;kr", "roccat;hyun;z;kr"
		};
		for (uint i = 0; i < ps.size(); i++) {
			auto p = &SearchPlayer(ps[i] DEBUGARGS);
			p->league = t_id;
			matches.matches[i / 2].players[i % 2] = p;
			frees.try_remove_vec(frees.koreans, p);
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_3/Code_A&action=edit&section=4", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaMatches(res, t_id, smatches);
		for (uint i = 0; i < smatches.size();i++) {
			auto &sm = smatches[i];
			if (i>0 && sm.timestamp < smatches[i-1].timestamp+3600*12) sm.timestamp = smatches[i-1].timestamp+3600;
			auto m = matches.GetMatch(sm.players);
			if (m) m->ReadOldResults(sm, prev_matches, upcoming_matches, matches.RoundOf());
			assert(m != NULL);
		}
	}

	void AcceptPrevBottom24(array<Player*, 24> &prev_bottom24, RayRand64 &rng)
	{
		RandomAdvancement<24, 24> t;
		for (uint i = 0; i<24; i++) {
			t.AddAdvancingPlayer(prev_bottom24[i], 1, 0, 0, 0, 0, rng);
		}
		for (uint i = 0; i<24; i++) {
			matches.matches[i].players[0] = t.advancing_players[i];
			//matches.matches[i].players[0] = t.advancing_players[i * 2];
			//matches.matches[i].players[2] = t.advancing_players[i * 2 + 1];
		}
	}

	void AcceptAdvancement(array<Player*, 24> &BtoA)
	{
		for (uint i = 0; i<24; i++) {
			matches.matches[i].players[1] = BtoA[i];
			//matches.matches[i].players[1] = BtoA[i * 2];
			//matches.matches[i].players[3] = BtoA[i * 2 + 1];
		}
	}

	void predict(Simulation &sim, RandomAdvancement<24, 24> &CtoP, FreePlayers &frees, RayRand64 &rng DEBUGARGSDEF)
	{
		matches.predict(sim, t_id, CtoP, rng DEBUGARGS);
		for (auto p : CtoP.falling_players) {
			frees.add_vec(frees.koreans, p);
		}
	}
};

class CodeSBase : public TournamentBase
{
public:
	Round<32, SwissGroup, 8, RandomAdvancement<16, 16>, true > ro32;
	Round<16, SwissGroup, 4, A1vsB2<8, 8>, true > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;

	//array<Player*, 32> ro32players;

	CodeSBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		quarterfinals.best_of = 5;
		semifinals.best_of = 7;
		finals.best_of = 7;
		ro32.points_for_placing[3] = 100;
		ro32.points_for_placing[2] = 150;
		ro16.points_for_placing[3] = 300;
		ro16.points_for_placing[2] = 400;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;
		finals.match_placing_to_tournament_placing[0] = 1;
	}

	void init_s1(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.match_placing_to_tournament_placing[3] = ro32.match_placing_to_tournament_placing[2] = 32;
		ro16.match_placing_to_tournament_placing[3] = ro16.match_placing_to_tournament_placing[2] = 16;
		quarterfinals.match_placing_to_tournament_placing[1] = 8;

		array<const char *, 32> ro32s = {
			//set matches
			";mc;p;kr", "kt;penguin;z;kr", "jin air;rogue;z;kr", "dpix;fantasy;t;kr",
			"mvp;marineking;t;kr", "tcm;soulkey;z;kr", "prime;yoda;t;kr", "yoe;san;p;kr",
			"mvp;gumiho;t;kr", "kt;life;z;kr", "myi;rain;p;kr", "mvp;blaze;p;kr",
			"kt;ty;t;kr", "SBENU;bomber;t;kr", "skt t1;dream;t;kr", "mvp;yonghwa;p;kr",
			"jin air;maru;t;kr", "SBENU;hack;t;kr", ";first;p;kr", "yoe;parting;p;kr",
			"liquid;taeja;t;kr", "cj entus;hero;p;kr", "jin air;terminator;p;kr", "SBENU;curious;z;kr",
			"skt t1;dark;z;kr", "acer;mma;t;kr", "mvp;seed;p;kr", "samsung;dear;p;kr",
			"axiom;heart;t;kr", "samsung;solar;z;kr", "kt;super;p;kr", "skt t1;innovation;t;kr"
		};
		for (uint i = 0; i<32; i++) {
			Player *p = &SearchPlayer(ro32s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
			frees.try_remove_vec(frees.koreans, p);
			//ro32players[i] = p;
		}
		//cerr << "here\n";
		for (uint i = 0; i<8; i++) {
			//cerr <<i<<"\n";
			string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_1/Code_S/Ro32&action=edit&section=T-" + ToString(i + 1), true, false);
			vector<ScrapedGroup> ro32_matches;
			ScrapeLiquipediaGroups(res, t_id, ro32_matches);
			//cerr << "ro32_matches.size()=="<<ro32_matches.size()<<"\n";
			auto *m = ro32.GetMatch(ro32_matches[0].players);
			if (m) m->ReadOldResults(ro32_matches[0], prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}

		array<const char *, 16> ro16s = {
			"skt t1;innovation;t;kr", "jin air;terminator;p;kr", "acer;mma;t;kr", "skt t1;dark;z;kr",
			"kt;life;z;kr", "prime;yoda;t;kr", "kt;ty;t;kr", "tcm;soulkey;z;kr",
			"myi;rain;p;kr", "jin air;rogue;z;kr", "skt t1;dream;t;kr", "samsung;solar;z;kr",
			"jin air;maru;t;kr", "yoe;parting;p;kr", ";mc;p;kr", "cj entus;hero;p;kr"
		};
		for (uint i = 0; i<16; i++) {
			Player *p = &SearchPlayer(ro16s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}
		vector<string> urls;
		for (uint i = 0; i<4; i++) {
			urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_1/Code_S/Ro16&action=edit&section=T-" + ToString(i + 1));
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint i = 0; i<4; i++) {
			//cerr <<i<<"\n";
			string res = ress[i];
			vector<ScrapedGroup> ro16_matches;
			ScrapeLiquipediaGroups(res, t_id, ro16_matches);
			//cerr << "ro32_matches.size()=="<<ro32_matches.size()<<"\n";
			auto *m = ro16.GetMatch(ro16_matches[0].players);
			if (m) m->ReadOldResults(ro16_matches[0], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		array<Player*, 8> top8;
		array<Player*, 24> bottom24;
		RandomAdvancement<6, 6> finalists;
		RayRand64 rng;
		predict_s1(sim, top8, bottom24, finalists, rng DEBUGARGS);
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_1/Code_S&action=edit&section=6", true, false);
		vector<ScrapedGroup> matches;
		ScrapeLiquipediaBracketsSimple(res, t_id, matches);
		for (uint i = 0; i < 4; i++) {
			auto *m = quarterfinals.GetMatch(matches[i].players);
			if (m) m->ReadOldResults(matches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict_s1(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4; i < 4 + 2 && i<matches.size(); i++) {
			auto *m = semifinals.GetMatch(matches[i].players);
			if (m) m->ReadOldResults(matches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict_s1(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1 && i<matches.size(); i++) {
			auto *m = finals.GetMatch(matches[i].players);
			if (m) m->ReadOldResults(matches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void init_s2(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.match_placing_to_tournament_placing[3] = ro32.match_placing_to_tournament_placing[2] = 32;
		ro16.match_placing_to_tournament_placing[3] = ro16.match_placing_to_tournament_placing[2] = 16;
		quarterfinals.match_placing_to_tournament_placing[1] = 8;

		array<const char *, 32> ro32s = {
			"samsung;solar;z;kr", "skt t1;sorry;t;kr", "myi;rain;p;kr", "mvp;gumiho;t;kr",
			"yoe;parting;p;kr", "cj entus;ragnarok;z;kr", "skt t1;dark;z;kr", ";myungsik;p;kr",
			"jin air;rogue;z;kr", "cj entus;trust;p;kr", "dpix;fantasy;t;kr", "skt t1;soo;z;kr",
			"skt t1;innovation;t;kr", ";creator;p;kr", "SBENU;bomber;t;kr", "mvp;blaze;p;kr",
			"kt;ty;t;kr", "liquid;hero;p;kr", "jin air;maru;t;kr", "cj entus;byul;z;kr",
			"kt;life;z;kr", "jin air;symbol;z;kr", "prime;yoda;t;kr", "cj entus;bbyong;t;kr",
			"cj entus;hero;p;kr", "dpix;patience;p;kr", "skt t1;dream;t;kr", "kt;flash;t;kr",
			"acer;mma;t;kr", "myi;sacsri;z;kr", "SBENU;curious;z;kr", "jin air;sos;p;kr"
		};
		for (uint i = 0; i < 32; i++) {
			Player *p = &SearchPlayer(ro32s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
			frees.try_remove_vec(frees.koreans, p);
		}

		array<const char *, 16> ro16s = {
			"kt;life;z;kr", ";myungsik;p;kr", "SBENU;bomber;t;kr", "jin air;maru;t;kr",
			"skt t1;innovation;t;kr", "myi;rain;p;kr", "jin air;sos;p;kr", "cj entus;bbyong;t;kr",
			"cj entus;hero;p;kr", "skt t1;dream;t;kr", "SBENU;curious;z;kr", "cj entus;byul;z;kr",
			"yoe;parting;p;kr", "mvp;gumiho;t;kr", "jin air;rogue;z;kr", "dpix;fantasy;t;kr"
		};
		for (uint i = 0; i < 16; i++) {
			Player *p = &SearchPlayer(ro16s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char *, 8> ro8s = {
			"jin air;maru;t;kr", "myi;rain;p;kr", "SBENU;curious;z;kr", "jin air;rogue;z;kr",
			";myungsik;p;kr", "jin air;sos;p;kr", "cj entus;byul;z;kr", "yoe;parting;p;kr"
		};
		for (uint i = 0; i < 8; i++) {
			Player *p = &SearchPlayer(ro8s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}
		//cerr << "here\n";
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_2/Code_S/Ro32&action=edit", true, false);
		vector<ScrapedGroup> ro32_matches;
		ScrapeLiquipediaGroups(res, t_id, ro32_matches);
		for (uint i = 0; i < 8; i++) {
			auto *m = ro32.GetMatch(ro32_matches[i].players);
			if (m) m->ReadOldResults(ro32_matches[i], prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_2/Code_S/Ro16&action=edit", true, false);
		vector<ScrapedGroup> ro16_matches;
		ScrapeLiquipediaGroups(res, t_id, ro16_matches);
		for (uint i = 0; i < 4; i++) {
			auto *m = ro16.GetMatch(ro16_matches[i].players);
			if (m) m->ReadOldResults(ro16_matches[i], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_2/Code_S&action=edit&section=6", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto *m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		array<Player*, 8> top8;
		array<Player*, 24> bottom24;
		array<Player*, 2> finalists;
		RayRand64 rng;
		predict_s2(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			auto *m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict_s2(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto *m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void init_s3(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.match_placing_to_tournament_placing[3] = ro32.match_placing_to_tournament_placing[2] = 32;
		ro16.match_placing_to_tournament_placing[3] = ro16.match_placing_to_tournament_placing[2] = 16;
		quarterfinals.match_placing_to_tournament_placing[1] = 8;

		array<const char *, 32> ro32s = {
			";myungsik;p;kr", "kt;zest;p;kr", "jin air;cure;t;kr", "samsung;solar;z;kr",
			"jin air;maru;t;kr", "skt t1;classic;p;kr", "cj entus;trust;p;kr", "sbenu;bomber;t;kr",
			"jin air;sos;p;kr", "samsung;stork;p;kr", "cj entus;bbyong;t;kr", "kt;life;z;kr",
			"myi;rain;p;kr", "mvp;losira;z;kr", "sbenu;hack;t;kr", "cj entus;hero;p;kr",
			"sbenu;curious;z;kr", "jin air;pigbaby;p;kr", "kt;flash;t;kr", "skt t1;innovation;t;kr",
			"cj entus;byul;z;kr", ";sleep;z;kr", "samsung;dear;p;kr", "dpix;fantasy;t;kr",
			"yoe;parting;p;kr", "cj entus;skyhigh;t;kr", "skt t1;dark;z;kr", "acer;mma;t;kr",
			"jin air;rogue;z;kr", "samsung;hurricane;p;kr", "skt t1;soo;z;kr", "mvp;gumiho;t;kr"
		};
		for (uint i = 0; i < 32; i++) {
			Player *p = &SearchPlayer(ro32s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
			frees.try_remove_vec(frees.koreans, p);
		}

		array<const char *, 16> ro16s = {
			"myi;rain;p;kr", "cj entus;bbyong;t;kr", "samsung;dear;p;kr", "jin air;rogue;z;kr",
			"cj entus;hero;p;kr", "jin air;maru;t;kr", "skt t1;classic;p;kr", "jin air;sos;p;kr",
			"cj entus;byul;z;kr", "kt;zest;p;kr", "skt t1;dark;z;kr", "samsung;solar;z;kr",
			"yoe;parting;p;kr", "mvp;gumiho;t;kr", "kt;flash;t;kr", "skt t1;innovation;t;kr"
		};
		for (uint i = 0; i < 16; i++) {
			Player *p = &SearchPlayer(ro16s[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_3/Code_S/Ro32&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro32;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro32);
		//sgroupsro32[1].players[1] = "Classic";//ugh, gonna have to fix some parsing to deal with the scores properly...
		//sgroupsro32[3].players[1] = "Rain";
		//sgroupsro32[3].players[0] = "herO";
		for (auto &g : sgroupsro32) {
			auto m = ro32.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_3/Code_S/Ro16&action=edit", false, false);
		vector<ScrapedGroup> sgroupsro16;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro16);
		for (auto &g : sgroupsro16) {
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_Global_StarCraft_II_League_Season_3/Code_S/Playoffs&action=edit&section=T-1", false, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		Simulation sim;
		array<Player*, 8> top8;
		array<Player*, 24> bottom24;
		array<Player*, 2> finalists;
		RayRand64 rng;
		predict_s3(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 0; i < 4 && i < smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		predict_s3(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4; i < 4+2 && i < smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict_s3(sim, top8, bottom24, finalists, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i < smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void AcceptPrevTop8(array<Player*, 8> &prev_top8, RayRand64 &rng)
	{
		RandomAdvancement<8, 8> t;
		for (uint i = 0; i<8; i++) {
			t.AddAdvancingPlayer(prev_top8[i], 1, 0, 0, 0, 0, rng);
		}
		for (uint i = 0; i<8; i++) {
			ro32.matches[i].players[0] = t.advancing_players[i];
		}
	}

	void AcceptAdvancement(array<Player*, 24> &CtoP)
	{
		for (int i = 0; i<8; i++) {
			ro32.matches[i].players[1] = CtoP[i * 3];
			ro32.matches[i].players[2] = CtoP[i * 3 + 1];
			ro32.matches[i].players[3] = CtoP[i * 3 + 2];
		}
	}

	/*void RandomizeRo32(RayRand64 &rng)
	{
	RandomAdvancement<32, 32> sortplayers;
	for (auto p : ro32players) {
	sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
	}
	ro32.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
	}*/

	void predict(Simulation &sim, array<Player*, 8> &top8, array<Player*, 24> &bottom24, array<Player*, 2> &finalists, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> Ro32toRo16;
		A1vsB2<8, 8> Ro16toRo8;
		StraightAdvancement<4, 4> Ro8toRo4;
		StraightAdvancement<2, 2> Ro4toRo2;
		StraightAdvancement<1, 1> finalsadv;

		ro32.predict(sim, t_id, Ro32toRo16, rng DEBUGARGS);
		ro16.AcceptAdvancements(Ro32toRo16.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, Ro16toRo8, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(Ro16toRo8.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, Ro8toRo4, rng DEBUGARGS);
		semifinals.AcceptAdvancements(Ro8toRo4.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, Ro4toRo2, rng DEBUGARGS);
		finals.AcceptAdvancements(Ro4toRo2.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);

		top8 = Ro16toRo8.advancing_players;
		for (uint i = 0; i<8; i++) {
			bottom24[i] = Ro32toRo16.falling_players[i * 2];
			bottom24[i + 8] = Ro32toRo16.falling_players[i * 2 + 1];
			bottom24[i + 16] = Ro16toRo8.falling_players[i];
		}
		/*for (uint i = 0; i<2; i++) {
			finalists.AddAdvancingPlayer(Ro4toRo2.advancing_players[i], 1, 1, 1, 1, 1, rng);
		}*/
		finalists[0] = finalsadv.advancing_players[0];
		finalists[1] = finalsadv.falling_players[0];
	}

	void predict_s1(Simulation &sim, array<Player*, 8> &top8, array<Player*, 24> &bottom24, RandomAdvancement<6, 6> &finalists, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> Ro32toRo16;
		A1vsB2<8, 8> Ro16toRo8;
		StraightAdvancement<4, 4> Ro8toRo4;
		StraightAdvancement<2, 2> Ro4toRo2;
		StraightAdvancement<1, 1> finalsadv;

		ro32.predict(sim, t_id, Ro32toRo16, rng DEBUGARGS);
		//ro16.AcceptAdvancements(Ro32toRo16.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, Ro16toRo8, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(Ro16toRo8.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, Ro8toRo4, rng DEBUGARGS);
		semifinals.AcceptAdvancements(Ro8toRo4.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, Ro4toRo2, rng DEBUGARGS);
		finals.AcceptAdvancements(Ro4toRo2.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);

		top8 = Ro16toRo8.advancing_players;
		for (uint i = 0; i<8; i++) {
			bottom24[i] = Ro32toRo16.falling_players[i * 2];
			bottom24[i + 8] = Ro32toRo16.falling_players[i * 2 + 1];
			bottom24[i + 16] = Ro16toRo8.falling_players[i];
		}
		for (uint i = 0; i<2; i++) {
			finalists.AddAdvancingPlayer(Ro4toRo2.advancing_players[i], 1, 1, 1, 1, 1, rng);
		}
	}

	void predict_s2(Simulation &sim, array<Player*, 8> &top8, array<Player*, 24> &bottom24, array<Player*, 2> &finalists, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> Ro32toRo16;
		A1vsB2<8, 8> Ro16toRo8;
		StraightAdvancement<4, 4> Ro8toRo4;
		StraightAdvancement<2, 2> Ro4toRo2;
		StraightAdvancement<1, 1> finalsadv;

		ro32.predict(sim, t_id, Ro32toRo16, rng DEBUGARGS);
		//ro16.AcceptAdvancements(Ro32toRo16.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, Ro16toRo8, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(Ro16toRo8.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, Ro8toRo4, rng DEBUGARGS);
		semifinals.AcceptAdvancements(Ro8toRo4.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, Ro4toRo2, rng DEBUGARGS);
		finals.AcceptAdvancements(Ro4toRo2.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);

		top8 = Ro16toRo8.advancing_players;
		for (uint i = 0; i<8; i++) {
			bottom24[i] = Ro32toRo16.falling_players[i * 2];
			bottom24[i + 8] = Ro32toRo16.falling_players[i * 2 + 1];
			bottom24[i + 16] = Ro16toRo8.falling_players[i];
		}
		/*for (uint i = 0; i<2; i++) {
		finalists.AddAdvancingPlayer(Ro4toRo2.advancing_players[i], 1, 1, 1, 1, 1, rng);
		}*/
		finalists[0] = finalsadv.advancing_players[0];
		finalists[1] = finalsadv.falling_players[0];
	}

	void predict_s3(Simulation &sim, array<Player*, 8> &top8, array<Player*, 24> &bottom24, array<Player*, 2> &finalists, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> Ro32toRo16;
		A1vsB2<8, 8> Ro16toRo8;
		StraightAdvancement<4, 4> Ro8toRo4;
		StraightAdvancement<2, 2> Ro4toRo2;
		StraightAdvancement<1, 1> finalsadv;

		ro32.predict(sim, t_id, Ro32toRo16, rng DEBUGARGS);
		//ro16.AcceptAdvancements(Ro32toRo16.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, Ro16toRo8, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(Ro16toRo8.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, Ro8toRo4, rng DEBUGARGS);
		semifinals.AcceptAdvancements(Ro8toRo4.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, Ro4toRo2, rng DEBUGARGS);
		finals.AcceptAdvancements(Ro4toRo2.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);

		top8 = Ro16toRo8.advancing_players;
		for (uint i = 0; i<8; i++) {
			bottom24[i] = Ro32toRo16.falling_players[i * 2];
			bottom24[i + 8] = Ro32toRo16.falling_players[i * 2 + 1];
			bottom24[i + 16] = Ro16toRo8.falling_players[i];
		}
		finalists[0] = finalsadv.advancing_players[0];
		finalists[1] = finalsadv.falling_players[0];
	}
};

class SPOTVBase : public TournamentBase//TournamentBase
{
public:
	Round<96, SwissGroup, 24, StraightGroupAdvancement<48, 48>, true > qualsro96;
	Round<48, SwissGroup, 12, RandomAdvancement<24, 24>, true > qualsro48;
	Round<24, SingleMatch, 12, RandomAdvancement<12, 12>, true > chalro24;

	Round<16, SwissGroup, 4, RandomAdvancement<8, 8>, true > mainro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;

	array<Player*, 4> set_chals;

	SPOTVBase(const char *name, uint TID) : TournamentBase(name, TID+1)
	{
		t_id = TID + 1;
		TournamentNames[TID] = string(name)+" Qualifiers";
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		qualsro96.ClearPlacings();
		qualsro48.ClearPlacings();
		qualsro48.match_placing_to_tournament_placing[0] = 1;
		qualsro48.match_placing_to_tournament_placing[1] = 1;
		qualsro96.best_of = 7;
		qualsro48.best_of = 7;

		chalro24.ClearPlacings();
		mainro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		finals.match_placing_to_tournament_placing[0] = 1;
		mainro16.points_for_placing[2] = 400;
		mainro16.points_for_placing[3] = 300;
		mainro16.match_placing_to_tournament_placing[2] = 12;
		mainro16.match_placing_to_tournament_placing[3] = 16;
		//mainro16.match_placing_to_tournament_placing[4]=20;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;

		chalro24.points_for_placing[1] = 50;
		chalro24.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		for (auto p : set_chals) p = NULL;
	}

	void AcceptAdvancement(array<Player*, 4> &prevtop4, array<Player*, 4> &chalseeds DEBUGARGSDEF)
	{
		for (uint i = 0; i<4; i++) {
			mainro16.matches[i].players[3] = prevtop4[i];
			set_chals[i] = chalseeds[i];
		}
	}

	void predict(Simulation &sim, array<Player*, 4> &top4, array<Player*, 4> &nextchal, RayRand64 &rng DEBUGARGSDEF)
	{
		array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		for (uint i = 0; i < 4; i++) {
			auto p = mainro16.matches[i].players[3];
			if (p) player_map[p->player_id] = 1;
		}
		for (uint i = 0; i < set_chals.size(); i++) {
			auto p = set_chals[i];
			if (p) player_map[p->player_id] = 1;
		}
		RandomAdvancement<96, 0> sortplayers;
		for (uint i = 0; i<96; i++) {
			Player *p = NULL;
			if (i<56) {
				//while(p==NULL) {
				p = good_koreans[rng.iRand64HQ() % good_koreans.size()];
				//}
			}
			else if (i<80) {
				p = good_players[rng.iRand64HQ() % good_players.size()];
			}
			else {
				//while(p==NULL || p->player_id==0) {
				p = &players[rng.iRand64HQ() % players.size()];
				//}
			}
			/*bool found=false;
			for(auto a : sortplayers.advancing_players) {
			if(a==p) {
			found=true;
			break;
			}
			}
			for(uint a=0;a<4;a++) {
			if(mainro16.matches[a].players[4]==p) {
			found=true;
			break;
			}
			if(chalro24.matches[a].players[1]==p) {
			found=true;
			break;
			}
			}
			if(found) {*/
			if (player_map[p->player_id] == 1) {
				i--;
				continue;
			}
			player_map[p->player_id] = 1;
			sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		StraightGroupAdvancement<48, 48> adv_ro96;
		RandomAdvancement<24, 24> adv_ro48;
		RandomAdvancement<12, 12> adv_ro24;
		RandomAdvancement<8, 8> adv_ro16;
		StraightAdvancement<4, 4> adv_ro8;
		StraightAdvancement<2, 2> adv_ro4;
		StraightAdvancement<1, 1> adv_finals;

		qualsro96.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		qualsro96.predict(sim, t_id - 1, adv_ro96, rng DEBUGARGS);
		qualsro48.AcceptAdvancements(adv_ro96.advancing_players DEBUGARGS);
		qualsro48.predict(sim, t_id - 1, adv_ro48, rng DEBUGARGS);

		array<Player*, 24> chals;
		for (uint i = 0; i<adv_ro48.advancing_players.size(); i++) {
			chals[i] = adv_ro48.advancing_players[i];
		}
		for (uint i = 0; i < set_chals.size(); i++) {
			if (set_chals[i]) chals[i * 2] = set_chals[i];
		}
		chalro24.AcceptAdvancements(chals DEBUGARGS);
		chalro24.predict(sim, t_id, adv_ro24, rng DEBUGARGS);

		for (uint i = 0; i<12; i++) {
			mainro16.matches[i / 3].players[i % 3] = adv_ro24.advancing_players[i];
		}
		mainro16.predict(sim, t_id, adv_ro16, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(adv_ro16.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, adv_ro8, rng DEBUGARGS);
		semifinals.AcceptAdvancements(adv_ro8.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, adv_ro4, rng DEBUGARGS);
		finals.AcceptAdvancements(adv_ro4.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, adv_finals, rng DEBUGARGS);
		top4 = adv_ro8.advancing_players;
		nextchal = adv_ro8.falling_players;
	}
};

class SPOTVS1 : public TournamentBase//TournamentBase
{
public:
	//Round<112, SwissGroup, 28, StraightGroupAdvancement<56, 56> > qualsro112;
	//Round<56, SwissGroup, 14, StraightGroupAdvancement<28, 28> > qualsro56;
	Round<32, SingleMatch, 16, RandomAdvancement<16, 16>, true > chalro32;

	Round<16, SwissGroup, 4, RandomAdvancement<8, 8>, true > mainro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;

	//array<Player*, 32> set_chals;

	SPOTVS1(const char *name, uint TID) : TournamentBase(name,TID+1)
	{
		t_id = TID + 1;
		TournamentNames[TID] = string(name) + " Qualifiers";
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		/*qualsro112.ClearPlacings();
		qualsro56.ClearPlacings();
		qualsro56.match_placing_to_tournament_placing[0] = 1;
		qualsro56.match_placing_to_tournament_placing[1] = 1;*/

		chalro32.ClearPlacings();
		mainro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		finals.match_placing_to_tournament_placing[0] = 1;
		mainro16.points_for_placing[2] = 400;
		mainro16.points_for_placing[3] = 300;
		//mainro16.points_for_placing[4]=300;
		mainro16.match_placing_to_tournament_placing[2] = 12;
		mainro16.match_placing_to_tournament_placing[3] = 16;
		mainro16.match_placing_to_tournament_placing[4] = 20;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;
		chalro32.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;
	}

	void init_s1(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);

		array<const char *, 32> chals_s = {
			//set matches
			"jin air;rogue;z;kr", "tcm;soulkey;z;kr", "skt t1;classic;p;kr", "jin air;cure;t;kr", "jin air;terminator;p;kr", "liquid;taeja;t;kr",
			"samsung;solar;z;kr", "skt t1;dream;t;kr", "kt;life;z;kr", "kt;flash;t;kr", "acer;scarlett;z;ca", "kt;ty;t;kr",
			"cj entus;skyhigh;t;kr", "mvp;gumiho;t;kr", "kt;stats;p;kr", "skt t1;innovation;t;kr", "dpix;patience;p;kr", "samsung;dear;p;kr",
			"jin air;maru;t;kr", "ig;jim;p;cn", "skt t1;dark;z;kr", "yoe;parting;p;kr", ";myungsik;p;kr", "yoe;san;p;kr",
			"kt;super;p;kr", "cj entus;byul;z;kr", "cj entus;hero;p;kr", "cj entus;bbyong;t;kr", "mvp;yonghwa;p;kr", "sbenu;leenock;z;kr",
			"dpix;fantasy;t;kr", "mvp;marineking;t;kr"
		};
		for (uint i = 0; i<32; i++) {
			Player *p = &SearchPlayer(chals_s[i] DEBUGARGS);
			if (p->league == 0 || p->league >= 30) p->league = t_id;
			//set_chals[i] = p;
			chalro32.matches[i / 2].players[i % 2] = p;
		}

		array<const char*, 16> sro16 = {
			"jin air;terminator;p;kr", "dpix;fantasy;t;kr", "sbenu;leenock;z;kr", "kt;super;p;kr",
			"kt;stats;p;kr", "skt t1;classic;p;kr", "kt;life;z;kr", "cj entus;hero;p;kr",
			"samsung;dear;p;kr", "mvp;gumiho;t;kr", "jin air;rogue;z;kr", ";myungsik;p;kr",
			"kt;ty;t;kr", "skt t1;dark;z;kr", "skt t1;dream;t;kr", "jin air;maru;t;kr"
		};
		for (uint i = 0; i<16; i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0 || p->league >= 30) p->league = t_id;
			mainro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 8> sro8 = {
			"sbenu;leenock;z;kr", "jin air;maru;t;kr", "kt;stats;p;kr", "jin air;rogue;z;kr",
			"samsung;dear;p;kr", "kt;life;z;kr", "skt t1;dream;t;kr", "kt;super;p;kr"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0 || p->league >= 30) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_1/Challenge&action=edit&section=3", true, false);
		vector<ScrapedGroup> chal_matches;
		ScrapeLiquipediaMatches(res, t_id, chal_matches);
		for (uint i = 0; i<16; i++) {
			auto *m = chalro32.GetMatch(chal_matches[i].players);
			if (m) m->ReadOldResults(chal_matches[i], prev_matches, upcoming_matches, chalro32.RoundOf());
			assert(m != NULL);
		}
		vector<string> urls;
		for (uint i = 0; i < 4; i++) {
			string url = "http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_1/Main_Event/Group_Stage&action=edit&section=T-" + ToString(i + 1);
			urls.push_back(url);
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint i = 0; i < 4; i++) {
			vector<ScrapedGroup> group;
			ScrapeLiquipediaGroups(ress[i], t_id, group);
			auto *m = mainro16.GetMatch(group[0].players);
			if (m) m->ReadOldResults(group[0], prev_matches, upcoming_matches, mainro16.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_1/Main_Event/Playoffs&action=edit&section=1", true, false);
		vector<ScrapedGroup> bracket_matches;
		ScrapeLiquipediaBracketsSimple(res, t_id, bracket_matches);
		for (uint i = 0; i<4; i++) {
			auto *m = quarterfinals.GetMatch(bracket_matches[i].players);
			if (m) m->ReadOldResults(bracket_matches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		array<Player*, 4> top4;
		array<Player*, 4> nextchal;
		set_predict(sim, top4, nextchal, rng DEBUGARGS);
		for (uint i = 4; i < 4 + 2 && i<bracket_matches.size(); i++) {
			auto *m = semifinals.GetMatch(bracket_matches[i].players);
			if (m) m->ReadOldResults(bracket_matches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		set_predict(sim, top4, nextchal, rng DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1 && i<bracket_matches.size(); i++) {
			auto *m = finals.GetMatch(bracket_matches[i].players);
			if (m) m->ReadOldResults(bracket_matches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void AcceptAdvancement(array<Player*, 4> &prevtop4, array<Player*, 4> &chalseeds DEBUGARGSDEF)
	{
		for (uint i = 0; i<4; i++) {
			//mainro16.matches[i].players[4] = prevtop4[i];
			//chalro32.matches[i].players[1] = chalseeds[i];
		}
	}

	void set_predict(Simulation &sim, array<Player*, 4> &top4, array<Player*, 4> &nextchal, RayRand64 &rng DEBUGARGSDEF)
	{
		/*RandomAdvancement<32,32> sort_chals;
		for(uint i=0;i<6;i++) sort_chals.advancing_players[i]=set_chals[i];
		for(uint i=6;i<32;i++) sort_chals.AddAdvancingPlayer(set_chals[i], 1,1,1,1,1,rng);*/

		RandomAdvancement<16, 16> adv_ro32;
		RandomAdvancement<8, 8> adv_ro16;
		StraightAdvancement<4, 4> adv_ro8;
		StraightAdvancement<2, 2> adv_ro4;
		StraightAdvancement<1, 1> adv_finals;

		//chalro32.AcceptAdvancements(set_chals DEBUGARGS);
		chalro32.predict(sim, t_id, adv_ro32, rng DEBUGARGS);

		/*for (uint i = 0; i<16; i++) {
		mainro16.matches[i / 4].players[i % 4] = adv_ro32.advancing_players[i];
		}*/
		mainro16.predict(sim, t_id, adv_ro16, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(adv_ro16.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, adv_ro8, rng DEBUGARGS);
		semifinals.AcceptAdvancements(adv_ro8.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, adv_ro4, rng DEBUGARGS);
		finals.AcceptAdvancements(adv_ro4.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, adv_finals, rng DEBUGARGS);
		top4 = adv_ro8.advancing_players;
		nextchal = adv_ro8.falling_players;
	}
};

class SPOTVS2 : public TournamentBase//TournamentBase
{
public:
	Round<24, SingleMatch, 12, RandomAdvancement<12, 12>, true > chalro24;

	Round<16, SwissGroup, 4, RandomAdvancement<8, 8>, true > mainro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;

	//array<Player*, 9> set_chals;
	//array<Player*, 15> set_quals;

	SPOTVS2(const char *name, uint TID) : TournamentBase(name, TID+1)
	{
		t_id = TID + 1;
		TournamentNames[TID] = string(name) + " Qualifiers";
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		chalro24.ClearPlacings();
		mainro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		finals.match_placing_to_tournament_placing[0] = 1;
		mainro16.points_for_placing[2] = 400;
		mainro16.points_for_placing[3] = 300;
		mainro16.match_placing_to_tournament_placing[2] = 12;
		mainro16.match_placing_to_tournament_placing[3] = 16;
		//mainro16.match_placing_to_tournament_placing[4]=20;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;

		chalro24.points_for_placing[1] = 50;
		chalro24.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		//for (auto p : set_chals) p = NULL;
	}

	void init_s2(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		chalro24.ResetPlacings();
		mainro16.ResetPlacings();

		/*array<const char*, 4> sro16 = {
		"kt;stats;p;kr", "jin air;maru;t;kr", "skt t1;dream;t;kr", "kt;life;z;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
		auto p = &SearchPlayer(sro16[i] DEBUGARGS);
		p->league = t_id;
		mainro16.matches[i].players[3] = p;
		}*/

		array<const char*, 24> schals = {
			"sbenu;leenock;z;kr", "kt;flash;t;kr", "jin air;rogue;z;kr", "samsung;hurricane;p;kr", "cj entus;hero;p;kr", "jin air;symbol;z;kr",
			"kt;super;p;kr", "skt t1;soo;z;kr", "jin air;cure;t;kr", "tcm;soulkey;z;kr", "jin air;trap;p;kr", "skt t1;dark;z;kr",
			"kt;zest;p;kr", "cj entus;bbyong;t;kr", "mvp;losira;z;kr", "roccat;hyun;z;kr", "skt t1;classic;p;kr", ";creator;p;kr",
			"skt t1;innovation;t;kr", "cj entus;byul;z;kr", "samsung;dear;p;kr", "jin air;sos;p;kr", "ig;macsed;p;cn", "yoe;parting;p;kr"
		};
		for (uint i = 0; i < schals.size(); i++) {
			auto p = &SearchPlayer(schals[i] DEBUGARGS);
			p->league = t_id;
			chalro24.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_2/Challenge&action=edit&section=4", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaMatches(res, t_id, smatches);
		for (uint i = 0; i < 12; i++) {
			auto m = chalro24.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chalro24.RoundOf());
		}

		array<const char*, 16> sro16 = {
			"jin air;maru;t;kr", "samsung;hurricane;p;kr", "kt;zest;p;kr", "sbenu;leenock;z;kr",
			"skt t1;dream;t;kr", "roccat;hyun;z;kr", "tcm;soulkey;z;kr", "skt t1;classic;p;kr",
			"kt;life;z;kr", "skt t1;soo;z;kr", "cj entus;byul;z;kr", "jin air;trap;p;kr",
			"kt;stats;p;kr", "samsung;dear;p;kr", "yoe;parting;p;kr", "cj entus;hero;p;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			p->league = t_id;
			mainro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 8> sro8 = {
			"cj entus;hero;p;kr", "jin air;maru;t;kr", "skt t1;dream;t;kr", "kt;life;z;kr",
			"cj entus;byul;z;kr", "skt t1;classic;p;kr", "kt;stats;p;kr", "kt;zest;p;kr",
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_2/Main_Event/Group%20Stage&action=edit", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (uint i = 0; i < 4; i++) {
			auto m = mainro16.GetMatch(sgroups[i].players);
			m->ReadOldResults(sgroups[i], prev_matches, upcoming_matches, mainro16.RoundOf());
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_2/Main_Event/Playoffs&action=edit&section=T-1", true, false);
		smatches.clear();
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			//if (smatches[i].timestamp == 0) smatches[i].timestamp = Timestamp("May 21, 2015 04:30 {{Abbr /CDT");
			auto m = quarterfinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		array<Player*, 4> top4;
		array<Player*, 4> nextchal;
		RayRand64 rng;
		predict(sim,top4, nextchal, rng DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			//if (smatches[i].timestamp == 0) smatches[i].timestamp = Timestamp("May 21, 2015 04:30 {{Abbr /CDT");
			auto m = semifinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, top4, nextchal, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			//if (smatches[i].timestamp == 0) smatches[i].timestamp = Timestamp("May 21, 2015 04:30 {{Abbr /CDT");
			auto m = finals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	/*void AcceptAdvancement(array<Player*, 4> &prevtop4, array<Player*, 4> &chalseeds DEBUGARGSDEF)
	{
	for (uint i = 0; i<4; i++) {
	mainro16.matches[i].players[3] = prevtop4[i];
	set_chals[i] = chalseeds[i];
	}
	}*/

	void predict(Simulation &sim, array<Player*, 4> &top4, array<Player*, 4> &nextchal, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<12, 12> adv_ro24;
		RandomAdvancement<8, 8> adv_ro16;
		StraightAdvancement<4, 4> adv_ro8;
		StraightAdvancement<2, 2> adv_ro4;
		StraightAdvancement<1, 1> adv_finals;

		//array<Player*, 24> chals;
		/*RandomAdvancement<24, 0> chals;
		for (uint i = 0; i<set_quals.size(); i++) {
		//chals[i*2] = set_quals[i];
		chals.AddAdvancingPlayer(set_quals[i], 0, 0, 0, 0, 0, rng);
		}
		for (uint i = 0; i < set_chals.size(); i++) {
		//if (set_chals[i]) chals[i * 2+1] = set_chals[i];
		chals.AddAdvancingPlayer(set_chals[i], 0, 0, 0, 0, 0, rng);
		}
		chalro24.AcceptAdvancements(chals.advancing_players DEBUGARGS);*/
		chalro24.predict(sim, t_id, adv_ro24, rng DEBUGARGS);

		sim.AddPlayerWCSPoints(players[145], -50);//MacSed doesn't get points cause invited

		/*for (uint i = 0; i<12; i++) {
		mainro16.matches[i / 3].players[i % 3] = adv_ro24.advancing_players[i];
		}*/
		mainro16.predict(sim, t_id, adv_ro16, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(adv_ro16.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, adv_ro8, rng DEBUGARGS);
		semifinals.AcceptAdvancements(adv_ro8.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, adv_ro4, rng DEBUGARGS);
		finals.AcceptAdvancements(adv_ro4.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, adv_finals, rng DEBUGARGS);
		top4 = adv_ro8.advancing_players;
		nextchal = adv_ro8.falling_players;
	}
};

class SPOTVS3 : public TournamentBase//TournamentBase
{
public:
	Round<24, SingleMatch, 12, RandomAdvancement<12, 12>, true > chalro24;
	
	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8>, true > mainro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;
	
	//array<Player*, 9> set_chals;
	//array<Player*, 24> set_chals;
	array<Player*, 2> invs;
	
	SPOTVS3(const char *name, uint TID) : TournamentBase(name, TID+1)
	{
		t_id = TID + 1;
		TournamentNames[TID] = string(name) + " Qualifiers";
	}
	
	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//chalro24.ClearPlacings();
		//mainro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		
		finals.match_placing_to_tournament_placing[0] = 1;
		mainro16.points_for_placing[2] = 400;
		mainro16.points_for_placing[3] = 300;
		mainro16.match_placing_to_tournament_placing[2] = 12;
		mainro16.match_placing_to_tournament_placing[3] = 16;
		//mainro16.match_placing_to_tournament_placing[4]=20;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;
		
		chalro24.points_for_placing[1] = 50;
		chalro24.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;
		
		array<const char*,24> schals = {
			";myungsik;p;kr", "samsung;journey;t;kr", "samsung;dear;p;kr", "skt t1;soo;z;kr", "jin air;maru;t;kr", "cj entus;sora;p;kr",
			"jin air;rogue;z;kr", "axiom;heart;t;kr", "cj entus;byul;z;kr", "samsung;shine;z;kr", "jin air;sos;p;kr", "samsung;solar;z;kr",
			"kt;zest;p;kr", "yoe;san;p;kr", "skt t1;dark;z;kr", "roccat;hyun;z;kr", "kt;ty;t;kr", "jin air;trap;p;kr",
			"skt t1;innovation;t;kr", "liquid;snute;z;no", "myi;rain;p;kr", "zoo;toodming;z;cn", "kt;life;z;kr", "sbenu;alive;t;kr"
		};
		for(uint i=0;i<schals.size();i++) {
			auto *p = &SearchPlayer(schals[i] DEBUGARGS);
			p->league = t_id;
			chalro24.matches[i/2].players[i%2] = p;
		}
		
		/*array<const char*,4> prev_top4 = {"skt t1;classic;p;kr", "kt;stats;p;kr", "cj entus;hero;p;kr", "skt t1;dream;t;kr"};
		for(uint i=0;i<prev_top4.size();i++) {
			auto *p = &SearchPlayer(prev_top4[i] DEBUGARGS);
			p->league = t_id;
			mainro16.matches[i].players[3] = p;
		}*/

		array<const char*, 16> sro16 = {
			"skt t1;classic;p;kr", "roccat;hyun;z;kr", "kt;ty;t;kr", "jin air;rogue;z;kr",
			"skt t1;dream;t;kr", ";myungsik;p;kr", "samsung;dear;p;kr", "myi;rain;p;kr",
			"cj entus;hero;p;kr", "skt t1;innovation;t;kr", "kt;life;z;kr", "jin air;sos;p;kr",
			"kt;stats;p;kr", "cj entus;byul;z;kr", "jin air;maru;t;kr", "kt;zest;p;kr"
		};
		for (uint i = 0; i<sro16.size(); i++) {
			auto *p = &SearchPlayer(sro16[i] DEBUGARGS);
			p->league = t_id;
			mainro16.matches[i/4].players[i%4] = p;
		}

		array<const char*, 2> sinvs = { "liquid;snute;z;no", "zoo;toodming;z;cn" };
		for (uint i = 0; i<sinvs.size(); i++) {
			auto *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_3/Challenge&action=edit&section=4", true, false);
		vector<ScrapedGroup> chalmatches;
		ScrapeLiquipediaMatches(res, t_id, chalmatches);
		for (uint i = 0; i < chalmatches.size(); i++) {
			auto m = chalro24.GetMatch(chalmatches[i].players);
			if (m) m->ReadOldResults(chalmatches[i], prev_matches, upcoming_matches, chalro24.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_3/Main_Event/Group_Stage&action=edit&section=T-1", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (auto &g : sgroups) {
			auto m = mainro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, mainro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		array<Player*, 4> top4;
		array<Player*, 4> nextchal;
		RayRand64 rng;
		predict(sim, top4, nextchal, rng DEBUGARGS);
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_StarCraft_II_StarLeague_Season_3/Main_Event/Playoffs&action=edit&section=T-1", false, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		/*if (smatches[0].players[0] == "Group Classic 1st") {
			smatches[0].players[0] = "tbd";
			quarterfinals.matches[0].players[0] = NULL;
		}
		if (smatches[2].players[1] == "Group Classic 2nd") {
			smatches[2].players[1] = "tbd";
			quarterfinals.matches[2].players[1] = NULL;
		}
		if (smatches[3].players[1] == "MyunNgSiK") smatches[3].players[1] = "myungsik";*/
		for (size_t i = 0;i<4;i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, top4, nextchal, rng DEBUGARGS);
		for (size_t i = 4; i<4+2 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, top4, nextchal, rng DEBUGARGS);
		for (size_t i = 4 + 2; i<4 + 2 + 1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}
	
	void init_s3(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
	}
	
	void predict(Simulation &sim, array<Player*, 4> &top4, array<Player*, 4> &nextchal, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<12, 12> adv_ro24;
		//RandomAdvancement<8, 8> adv_ro16;
		StraightGroupAdvancement<8, 8> adv_ro16;
		StraightAdvancement<4, 4> adv_ro8;
		StraightAdvancement<2, 2> adv_ro4;
		StraightAdvancement<1, 1> adv_finals;
		
		chalro24.predict(sim, t_id, adv_ro24, rng DEBUGARGS);
		
		for (auto p1 : adv_ro24.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerWCSPoints(*p1, -50);
					break;
				}
			}
		}
		/*for (uint i = 0; i<12; i++) {
			mainro16.matches[i / 3].players[i % 3] = adv_ro24.advancing_players[i];
		}*/
		mainro16.predict(sim, t_id, adv_ro16, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(adv_ro16.advancing_players DEBUGARGS);
		quarterfinals.matches[0].players[0] = adv_ro16.advancing_players[0];//rogue
		quarterfinals.matches[0].players[1] = adv_ro16.advancing_players[1];//zest
		quarterfinals.matches[1].players[0] = adv_ro16.advancing_players[6];//byul
		quarterfinals.matches[1].players[1] = adv_ro16.advancing_players[3];//inno
		quarterfinals.matches[2].players[0] = adv_ro16.advancing_players[2];//rain
		quarterfinals.matches[2].players[1] = adv_ro16.advancing_players[7];//hyun
		quarterfinals.matches[3].players[0] = adv_ro16.advancing_players[4];//hero
		quarterfinals.matches[3].players[1] = adv_ro16.advancing_players[5];//myungsik

		quarterfinals.predict(sim, t_id, adv_ro8, rng DEBUGARGS);
		semifinals.AcceptAdvancements(adv_ro8.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, adv_ro4, rng DEBUGARGS);
		finals.AcceptAdvancements(adv_ro4.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, adv_finals, rng DEBUGARGS);
		//top4 = adv_ro8.advancing_players;
		top4[0] = adv_finals.advancing_players[0];
		top4[1] = adv_finals.falling_players[0];
		top4[2] = adv_ro4.falling_players[0];
		top4[3] = adv_ro4.falling_players[1];
		nextchal = adv_ro8.falling_players;
	}
};

class WCSQualifiersBase : public TournamentBase
{
public:
	Round<112, SwissGroup, 28, StraightGroupAdvancement<56, 56> > ro112EU;
	Round<56, SwissGroup, 14, StraightGroupAdvancement<28, 28> > ro56EU;
	Round<28, SwissGroup, 7, MixedAdvancement<14, 14> > ro28EU;

	Round<32, SwissGroup, 8, StraightGroupAdvancement<16, 16> > ro32NA;
	Round<16, SwissGroup, 4, MixedAdvancement<8, 8> > ro16NA;

	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8> > ro16CN;
	Round<8, SwissGroup, 2, MixedAdvancement<4, 4> > ro8CN;

	Round<8, SwissGroup, 2, StraightGroupAdvancement<4, 4> > ro8o[3];
	Round<4, SwissGroup, 1, MixedAdvancement<2, 2> > ro4o[3];

	WCSQualifiersBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(DEBUGARGDEF)
	{
		ro112EU.ClearPlacings();
		ro56EU.ClearPlacings();
		ro28EU.ClearPlacings();
		ro28EU.match_placing_to_tournament_placing[0] = 1;
		ro28EU.match_placing_to_tournament_placing[1] = 1;
		ro32NA.ClearPlacings();
		ro16NA.ClearPlacings();
		ro16NA.match_placing_to_tournament_placing[0] = 1;
		ro16NA.match_placing_to_tournament_placing[1] = 1;
		ro16CN.ClearPlacings();
		ro8CN.ClearPlacings();
		ro8CN.match_placing_to_tournament_placing[0] = 1;
		ro8CN.match_placing_to_tournament_placing[1] = 1;
		for (uint i = 0; i<3; i++) {
			ro8o[i].ClearPlacings();
			ro4o[i].ClearPlacings();
			ro4o[i].match_placing_to_tournament_placing[0] = 1;
			ro4o[i].match_placing_to_tournament_placing[1] = 1;
			ro8o[i].best_of = 7;
			ro4o[i].best_of = 7;
		}
		ro112EU.best_of = 7;
		ro56EU.best_of = 7;
		ro28EU.best_of = 7;
		ro32NA.best_of = 7;
		ro16NA.best_of = 7;
		ro16CN.best_of = 7;
		ro8CN.best_of = 7;
	}

	void init_s1(DEBUGARGDEF)
	{
		init(DEBUGARG);
		ro112EU.ClearPlacings();
		ro56EU.ClearPlacings();
		ro28EU.ClearPlacings();
		ro32NA.ClearPlacings();
		ro16NA.ClearPlacings();
		ro16CN.ClearPlacings();
		ro8CN.ClearPlacings();
		for (uint i = 0; i<3; i++) {
			ro8o[i].ClearPlacings();
			ro4o[i].ClearPlacings();
		}
	}

	void init_s2(DEBUGARGDEF)
	{
		init(DEBUGARG);
		ro16CN.ClearPlacings();
		ro8CN.ClearPlacings();
		ro28EU.ClearPlacings();
		ro4o[1].ClearPlacings();//sea
		ro16NA.ClearPlacings();
		ro4o[2].ClearPlacings();//tw
	}

	void init_s3(DEBUGARGDEF)
	{
		init(DEBUGARG);
		ro112EU.ClearPlacings();
		ro56EU.ClearPlacings();
		ro28EU.ClearPlacings();
		ro32NA.ClearPlacings();
		ro16NA.ClearPlacings();
		ro16CN.ClearPlacings();
		ro8CN.ClearPlacings();
		for (uint i = 0; i<3; i++) {
			ro8o[i].ClearPlacings();
			ro4o[i].ClearPlacings();
		}
	}

	void predict(Simulation &sim, FreePlayers &frees, array<Player*, 32> &QtoC, RayRand64 &rng DEBUGARGSDEF)
	{
		uint outslot = 0;
		MixedAdvancement<14, 14> EUtoC;
		MixedAdvancement<8, 8> NAtoC;
		MixedAdvancement<4, 4> CNtoC;
		array<MixedAdvancement<2, 2>, 3> OtoC;
		for (uint i = 0; i<56; i++) {
			size_t slot = 0;
			assert(frees.europeans.size()>0);
			slot = rng.iRand64HQ() % frees.europeans.size();
			for (uint c = 0; c<10 && i % 4 == 0 && frees.europeans[slot]->ratings[R]<1300; c++) {
				slot = rng.iRand64HQ() % frees.europeans.size();
			}
			ro56EU.matches[i / 4].players[i % 4] = frees.europeans[slot];
			frees.europeans[slot] = frees.europeans.back();
			frees.europeans.pop_back();
		}

		StraightGroupAdvancement<28, 28> adv_eu;
		ro56EU.PredictNoOldResults(sim, t_id, adv_eu, rng DEBUGARGS);
		ro28EU.AcceptAdvancements(adv_eu.advancing_players DEBUGARGS);
		ro28EU.PredictNoOldResults(sim, t_id, EUtoC, rng DEBUGARGS);

		for (auto p : adv_eu.falling_players) frees.add_vec(frees.europeans, p);
		for (auto p : EUtoC.falling_players) frees.add_vec(frees.europeans, p);
		for (auto p : EUtoC.advancing_players) QtoC[outslot++] = p;

		//---------------------
		for (uint i = 0; i<32; i++) {
			size_t slot = 0;
			assert(frees.northamericans.size()>0);
			slot = rng.iRand64HQ() % frees.northamericans.size();
			for (uint c = 0; c<10 && i % 8 == 0 && frees.northamericans[slot]->ratings[R]<1300; c++) {
				slot = rng.iRand64HQ() % frees.northamericans.size();
			}
			ro32NA.matches[i / 4].players[i % 4] = frees.northamericans[slot];
			frees.northamericans[slot] = frees.northamericans.back();
			frees.northamericans.pop_back();
		}

		StraightGroupAdvancement<16, 16> adv_na;
		ro32NA.PredictNoOldResults(sim, t_id, adv_na, rng DEBUGARGS);
		ro16NA.AcceptAdvancements(adv_na.advancing_players DEBUGARGS);
		ro16NA.PredictNoOldResults(sim, t_id, NAtoC, rng DEBUGARGS);

		for (auto p : adv_na.falling_players) frees.add_vec(frees.northamericans, p);
		for (auto p : NAtoC.falling_players) frees.add_vec(frees.northamericans, p);
		for (auto p : NAtoC.advancing_players) QtoC[outslot++] = p;

		//---------------------
		for (uint i = 0; i<16; i++) {
			size_t slot = 0;
			assert(frees.chinese.size()>0);
			slot = rng.iRand64HQ() % frees.chinese.size();
			for (uint c = 0; c<10 && i % 8 == 0 && frees.chinese[slot]->ratings[R]<1300; c++) {
				slot = rng.iRand64HQ() % frees.chinese.size();
			}
			ro16CN.matches[i / 4].players[i % 4] = frees.chinese[slot];
			frees.chinese[slot] = frees.chinese.back();
			frees.chinese.pop_back();
		}

		StraightGroupAdvancement<8, 8> adv_cn;
		ro16CN.PredictNoOldResults(sim, t_id, adv_cn, rng DEBUGARGS);
		ro8CN.AcceptAdvancements(adv_cn.advancing_players DEBUGARGS);
		ro8CN.PredictNoOldResults(sim, t_id, CNtoC, rng DEBUGARGS);

		for (auto p : adv_cn.falling_players) frees.add_vec(frees.chinese, p);
		for (auto p : CNtoC.falling_players) frees.add_vec(frees.chinese, p);
		for (auto p : CNtoC.advancing_players) QtoC[outslot++] = p;

		for (uint q = 0; q < 3; q++) {
			vector<Player*> *vec = &frees.latinamericans;
			if (q == 1) vec = &frees.seas;
			if (q == 2) vec = &frees.taiwanese;
			vector<Player*> &v = *vec;
			for (uint i = 0; i < 8; i++) {
				size_t slot = 0;
				assert(v.size() > 0);
				slot = rng.iRand64HQ() % v.size();
				for (uint c = 0; c<10 && i % 4 == 0 && v[slot]->ratings[R]<1300; c++) {
					slot = rng.iRand64HQ() % v.size();
				}
				ro8o[q].matches[i / 4].players[i % 4] = v[slot];
				v[slot] = v.back();
				v.pop_back();
			}

			StraightGroupAdvancement<4, 4> adv_o;
			ro8o[q].PredictNoOldResults(sim, t_id, adv_o, rng DEBUGARGS);
			ro4o[q].AcceptAdvancements(adv_o.advancing_players DEBUGARGS);
			ro4o[q].PredictNoOldResults(sim, t_id, OtoC[q], rng DEBUGARGS);

			for (auto p : adv_o.falling_players) frees.add_vec(v, p);
			for (auto p : OtoC[q].falling_players) frees.add_vec(v, p);
			for (auto p : OtoC[q].advancing_players) QtoC[outslot++] = p;
		}
	}
};

class WCSChallengerBase : public TournamentBase
{
public:
	Round<64, SingleMatch, 32, StraightAdvancement<32, 32>, true > ro64;

	WCSChallengerBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		ro64.best_of = 5;
		ro64.points_for_placing[1] = 50;
		ro64.ClearPlacings();
		ro64.match_placing_to_tournament_placing[0] = 1;
	}

	void AcceptAdvancements(array<Player*, 32> &prevCtoC, array<Player*, 32> &QtoC, Player *champ, FreePlayers &frees, RayRand64 &rng DEBUGARGSDEF)
	{
		/*RandomAdvancement<32, 0> prevsort;
		RandomAdvancement<32, 0> newsort;
		for (uint i = 0; i < 32; i++) {
		prevsort.AddAdvancingPlayer(prevCtoC[i], 1, 1, 1, 1, 1, rng);
		newsort.AddAdvancingPlayer(QtoC[i], 1, 1, 1, 1, 1, rng);
		}*/ //this ruins the regional pairing
		/*for(uint i=0;i<32;i++) {
		ro64.matches[i].players[0]=prevsort.advancing_players[i];
		ro64.matches[i].players[1]=newsort.advancing_players[i];
		}*/
		for (uint i = 0; i<32; i++) {
			auto p = prevCtoC[i];
			auto n = QtoC[i];
			//auto p = prevsort.advancing_players[i];
			//auto n = newsort.advancing_players[i];
			ro64.matches[i].players[0] = p;// prevCtoC[i];
			ro64.matches[i].players[1] = n;// QtoC[i];
			/*if (p == champ) {
			ro64.matches[i].players[1] = &players[0];
			frees.add(n);
			}*/
		}
		uint walkoverslot = 100;
		uint champslot = 0;
		for (uint i = 0; i < 32; i++) {
			if (ro64.matches[i].players[0] == champ) {
				champslot = i;
			}
			if (ro64.matches[i].players[1]->player_id == 0) {
				walkoverslot = i;
			}
		}
		if (walkoverslot == 100) {
			walkoverslot = champslot;
			//frees.add(ro64.matches[walkoverslot].players[1]);
		}

		auto p = ro64.matches[walkoverslot].players[0];
		ro64.matches[walkoverslot].players[0] = champ;
		ro64.matches[champslot].players[0] = p;
		frees.add(ro64.matches[walkoverslot].players[1]);
		ro64.matches[walkoverslot].players[1] = &players[0];
	}

	void predict(Simulation &sim, FreePlayers &frees, array<Player*, 32> &CtoC, RandomAdvancement<32, 32> &CtoP, RayRand64 &rng DEBUGARGSDEF)
	{
		StraightAdvancement<32, 32> sadv;
		ro64.predict(sim, t_id, sadv, rng DEBUGARGS);
		CtoC = sadv.advancing_players;
		for (auto p : sadv.falling_players) frees.add(p);
		for (auto p : sadv.advancing_players) CtoP.AddAdvancingPlayer(p, 1, 1, 1, 1, 32, rng);
	}
};

class WCSPremierBase : public TournamentBase
{
public:
	Round<32, SwissGroup, 8, RandomAdvancement<16, 16>, true > ro32;
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8>, true > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4>, true > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2>, true > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1>, true > finals;

	WCSPremierBase(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		quarterfinals.best_of = 5;
		semifinals.best_of = 7;
		finals.best_of = 7;
		ro32.points_for_placing[3] = 100;
		ro32.points_for_placing[2] = 150;
		ro16.points_for_placing[3] = 300;
		ro16.points_for_placing[2] = 400;
		quarterfinals.points_for_placing[1] = 600;
		semifinals.points_for_placing[1] = 900;
		finals.points_for_placing[1] = 1250;
		finals.points_for_placing[0] = 2000;
		finals.match_placing_to_tournament_placing[0] = 1;
	}

	void init_s1(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		//ro32.ResetPlacings();
		//ro16.ResetPlacings();
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();

		array<const char*, 32> sro32 = {
			"ig;xigua;z;cn", "ta;astrea;p;us", "at;uthermal;t;nl", "liquid;snute;z;no",
			"root;kelazhur;t;br", "liquid;tlo;z;de", "cm storm;polt;t;kr", "ig;jim;p;cn",
			"liquid;bunny;t;dk", ";major;t;mx", "envy;violet;z;kr", "liquid;mana;p;pl",
			";happy;t;ru", "x5;pig;z;au", ";kane;z;ca", ";targa;z;no",
			"f3;puck;p;us", ";firecake;z;fr", "hka;sen;z;tw", "millenium;forgg;t;kr",
			"eg;suppy;z;us", "myi;serral;z;fi", "yoe;has;p;tw", "property;naniwa;p;se",
			"property;morrow;t;se", "root;hydra;z;kr", ";showtime;p;de", "ig;iasonu;z;cn",
			";iaguz;t;au", "ig;macsed;p;cn", ";welmu;p;fi", "eg;huk;p;ca",
		};
		for (uint i = 0; i < sro32.size(); i++) {
			Player *p = &SearchPlayer(sro32[i] DEBUGARGS);
			p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 16> sro16 = {
			"liquid;tlo;z;de", "millenium;forgg;t;kr", "ig;xigua;z;cn", "myi;serral;z;fi",
			"x5;pig;z;au", ";showtime;p;de", "ig;macsed;p;cn", "liquid;snute;z;no",
			"root;hydra;z;kr", "liquid;mana;p;pl", ";firecake;z;fr", "cm storm;polt;t;kr",
			"yoe;has;p;tw", ";kane;z;ca", "liquid;bunny;t;dk", ";welmu;p;fi"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char *, 8> sro8 = {
			"root;hydra;z;kr", "liquid;tlo;z;de", ";kane;z;ca", ";showtime;p;de",
			"liquid;snute;z;no", "liquid;bunny;t;dk", "millenium;forgg;t;kr", "cm storm;polt;t;kr"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		vector<string> urls;
		for (uint i = 0; i < 8; i++) {
			string url = "http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Premier/Ro32&action=edit&section=T-" + ToString(i + 1);
			urls.push_back(url);
		}
		for (uint i = 0; i < 4; i++) {
			string url = "http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Premier/Ro16&action=edit&section=T-" + ToString(i + 1);
			urls.push_back(url);
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint i = 0; i < 8; i++) {
			vector<ScrapedGroup> group;
			ScrapeLiquipediaGroups(ress[i], t_id, group);
			auto *m = ro32.GetMatch(group[0].players);
			if (m) m->ReadOldResults(group[0], prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}
		for (uint i = 0; i < 4; i++) {
			vector<ScrapedGroup> group;
			ScrapeLiquipediaGroups(ress[i + 8], t_id, group);
			auto *m = ro16.GetMatch(group[0].players);
			if (m) m->ReadOldResults(group[0], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Premier&action=edit&section=6", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		Player *champ;
		predict_s1(sim, rng, champ DEBUGARGS);
		for (uint i = 4; i < 4 + 2; i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict_s1(sim, rng, champ DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1; i++) {
			auto m = finals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void init_s2(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.ResetPlacings();
		ro16.ResetPlacings();
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();

		array<const char*, 32> sro32 = {
			"ig;jim;p;cn", ";kane;z;ca", "liquid;bunny;t;dk", ";petraeus;z;nz",
			"ig;iasonu;z;cn", "eg;xenocider;t;us", "invasion;harstem;p;nl", "silicon;nxz;z;au",
			"bheart;mystery;p;cn", "liquid;ret;z;nl", "envy;violet;z;kr", "millenium;marinelord;t;fr",
			"zoo;toodming;z;cn", ";stardust;p;kr", "millenium;forgg;t;kr", "hka;sen;z;tw",
			"millenium;lilbow;p;fr", "root;kelazhur;t;br", "root;hydra;z;kr", "liquid;snute;z;no",
			"tes;elazer;z;pl", ";major juan;t;mx", "cm storm;polt;t;kr", "liquid;tlo;z;de",
			"liquid;mana;p;pl", ";iaguz;t;au", "root;masa;t;ca", ";firecake;z;fr",
			"planetkey;gungfubanda;p;de", "yoe;has;p;tw", "eg;jaedong;z;kr", ";elfi;p;fi"
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 16> sro16 = {
			"millenium;forgg;t;kr", "planetkey;gungfubanda;p;de", "liquid;bunny;t;dk", "root;hydra;z;kr",
			"invasion;harstem;p;nl", "cm storm;polt;t;kr", ";firecake;z;fr", "millenium;marinelord;t;fr",
			"millenium;lilbow;p;fr", "ig;iasonu;z;cn", "liquid;tlo;z;de", ";iaguz;t;au",
			"liquid;ret;z;nl", ";stardust;p;kr", "eg;jaedong;z;kr", ";petraeus;z;nz"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 8> sro8 = {
			";stardust;p;kr", "liquid;bunny;t;dk", "millenium;lilbow;p;fr", "millenium;marinelord;t;fr",
			"root;hydra;z;kr", "eg;jaedong;z;kr", "cm storm;polt;t;kr", "liquid;tlo;z;de"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Premier/Ro32&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro32;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro32);
		for (uint i = 0; i < 8; i++) {
			auto m = ro32.GetMatch(sgroupsro32[i].players);
			assert(m != NULL);
			m->ReadOldResults(sgroupsro32[i], prev_matches, upcoming_matches, ro32.RoundOf());
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Premier/Ro16&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro16;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro16);
		for (uint i = 0; i < 4; i++) {
			auto m = ro16.GetMatch(sgroupsro16[i].players);
			assert(m != NULL);
			m->ReadOldResults(sgroupsro16[i], prev_matches, upcoming_matches, ro16.RoundOf());
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Premier/Playoffs&action=edit&section=T-1", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		Player *champ;
		predict_s2(sim, rng, champ DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict_s2(sim, rng, champ DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void init_s3(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.ResetPlacings();
		ro16.ResetPlacings();
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();

		array<const char*, 32> sro32 = {
			"invasion;harstem;p;nl", ";major;t;mx", "cm storm;polt;t;kr", ";petraeus;z;nz",
			";elfi;p;fi", ";starbuck;z;si", "root;hydra;z;kr", "root;kelazhur;t;br",
			";showtime;p;de", "hka;sen;z;tw", "eg;jaedong;z;kr", "mouz;heromarine;t;de",
			"millenium;marinelord;t;fr", "root;masa;t;ca", "yoe;has;p;tw", ";iaguz;t;au",
			"millenium;lilbow;p;fr", ";neeb;p;us", "x-team;shana;t;cn", "planetkey;gungfubanda;p;de",
			"property;zanster;z;se", ";kane;z;ca", "ig;iasonu;z;cn", "millenium;forgg;t;kr",
			"liquid;snute;z;no", "ig;jim;p;cn", ";hitman;p;us", "tes;elazer;z;pl",
			"liquid;tlo;z;de", "envy;violet;z;kr", "ig;xigua;z;cn", "liquid;mana;p;pl"			
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
		}
		
		array<const char*, 16> sro16 = {
			"millenium;marinelord;t;fr", ";elfi;p;fi", "liquid;mana;p;pl", "property;zanster;z;se",
			"liquid;snute;z;no", "hka;sen;z;tw", "planetkey;gungfubanda;p;de", ";petraeus;z;nz",
			"cm storm;polt;t;kr", "envy;violet;z;kr", "root;hydra;z;kr", "tes;elazer;z;pl",
			"millenium;forgg;t;kr", ";iaguz;t;au", "eg;jaedong;z;kr", "millenium;lilbow;p;fr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 8> sro8 = {
			"property;zanster;z;se", "planetkey;gungfubanda;p;de",
			"millenium;lilbow;p;fr", "root;hydra;z;kr",
			"cm storm;polt;t;kr", "millenium;forgg;t;kr",
			";petraeus;z;nz", "liquid;mana;p;pl"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}
		
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Premier/Ro32&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro32;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro32);
		for (auto &g : sgroupsro32) {
			auto m = ro32.GetMatch(g.players);
			assert(m != NULL);
			m->ReadOldResults(g, prev_matches, upcoming_matches, ro32.RoundOf());
		}
		
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Premier/Ro16&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro16;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro16);
		for (auto &g : sgroupsro16) {
			auto m = ro16.GetMatch(g.players);
			assert(m != NULL);
			m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Premier&action=edit&section=6", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			assert(m != NULL);
			m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		Player* champ;
		predict_s3(sim, rng, champ DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			assert(m != NULL);
			m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict_s3(sim, rng, champ DEBUGARGS);
		for (uint i = 4+2; i < 4+2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			assert(m != NULL);
			m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void AcceptAdvancements(RandomAdvancement<32, 32> &CtoP DEBUGARGSDEF)
	{
		ro32.AcceptAdvancements(CtoP.advancing_players DEBUGARGS);
	}

	void predict(Simulation &sim, RayRand64 &rng, Player * &champ DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> ro32_adv;
		RandomAdvancement<8, 8> ro16_adv;
		StraightAdvancement<4, 4> ro8_adv;
		StraightAdvancement<2, 2> ro4_adv;
		StraightAdvancement<1, 1> ro2_adv;
		ro32.predict(sim, t_id, ro32_adv, rng DEBUGARGS);
		ro16.AcceptAdvancements(ro32_adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16_adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16_adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, ro8_adv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(ro8_adv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, ro4_adv, rng DEBUGARGS);
		finals.AcceptAdvancements(ro4_adv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, ro2_adv, rng DEBUGARGS);
		champ = ro2_adv.advancing_players[0];
	}

	void predict_s1(Simulation &sim, RayRand64 &rng, Player * &champ DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> ro32_adv;
		RandomAdvancement<8, 8> ro16_adv;
		StraightAdvancement<4, 4> ro8_adv;
		StraightAdvancement<2, 2> ro4_adv;
		StraightAdvancement<1, 1> ro2_adv;
		ro32.predict(sim, t_id, ro32_adv, rng DEBUGARGS);
		//ro16.AcceptAdvancements(ro32_adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16_adv, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(ro16_adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, ro8_adv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(ro8_adv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, ro4_adv, rng DEBUGARGS);
		finals.AcceptAdvancements(ro4_adv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, ro2_adv, rng DEBUGARGS);
		champ = ro2_adv.advancing_players[0];
	}

	void predict_s2(Simulation &sim, RayRand64 &rng, Player * &champ DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> ro32_adv;
		RandomAdvancement<8, 8> ro16_adv;
		StraightAdvancement<4, 4> ro8_adv;
		StraightAdvancement<2, 2> ro4_adv;
		StraightAdvancement<1, 1> ro2_adv;
		ro32.predict(sim, t_id, ro32_adv, rng DEBUGARGS);
		//ro16.AcceptAdvancements(ro32_adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16_adv, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(ro16_adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, ro8_adv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(ro8_adv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, ro4_adv, rng DEBUGARGS);
		finals.AcceptAdvancements(ro4_adv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, ro2_adv, rng DEBUGARGS);
		champ = ro2_adv.advancing_players[0];
	}
	
	void predict_s3(Simulation &sim, RayRand64 &rng, Player * &champ DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> ro32_adv;
		RandomAdvancement<8, 8> ro16_adv;
		StraightAdvancement<4, 4> ro8_adv;
		StraightAdvancement<2, 2> ro4_adv;
		StraightAdvancement<1, 1> ro2_adv;
		ro32.predict(sim, t_id, ro32_adv, rng DEBUGARGS);
		//ro16.AcceptAdvancements(ro32_adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16_adv, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(ro16_adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, ro8_adv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(ro8_adv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, ro4_adv, rng DEBUGARGS);
		finals.AcceptAdvancements(ro4_adv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, ro2_adv, rng DEBUGARGS);
		champ = ro2_adv.advancing_players[0];
	}
};

class WCSSeasonAdvancements
{
public:
	array<Player*, 8> KR_top8;
	array<Player*, 24> KR_bottom24;
	array<Player*, 32> CtoC;
	array<Player*, 4> SPOTV_top4;
	array<Player*, 4> SPOTV_chalseeds;
	array<Player*, 2> gsl_finalists;
	Player* champ;
};

class WCSBase : public TournamentBase
{
public:
	KRQualifiersBase codeb;
	CodeABase codea;
	CodeSBase codes;

	WCSQualifiersBase quals;
	WCSChallengerBase chal;
	WCSPremierBase prem;

	//SPOTVBase spotv;

	WCSBase(uint season) : TournamentBase("WCSBase", 0),
		codeb(("Code B "+ToString(season)).c_str(), (season - 1) * 12 + 1), codea(("Code A "+ToString(season)).c_str(), (season - 1) * 12 + 2), codes(("Code S "+ToString(season)).c_str(), (season - 1) * 12 + 3),
		quals(("WCS Qualifiers "+ToString(season)).c_str(), (season - 1) * 12 + 4), chal(("WCS Challenger "+ToString(season)).c_str(), (season - 1) * 12 + 5), prem(("WCS Premier "+ToString(season)).c_str(), (season - 1) * 12 + 6)
		/*, spotv("SPOTV", (season-1)*12+7)*/
	{//SPOTV counts as 2 tournaments cause of the qualifiers...
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		codeb.init(upcoming_matches DEBUGARGS);
		codea.init(prev_matches, upcoming_matches DEBUGARGS);
		codes.init(prev_matches, upcoming_matches DEBUGARGS);

		quals.init(DEBUGARG);
		chal.init(prev_matches, upcoming_matches DEBUGARGS);
		prem.init(prev_matches, upcoming_matches DEBUGARGS);

		//spotv.init(prev_matches, upcoming_matches DEBUGARGS);
	}

	void predict(Simulation &sim, FreePlayers &frees, WCSSeasonAdvancements &prev_advs, WCSSeasonAdvancements &next_advs, RayRand64 &rng DEBUGARGSDEF)
	{
		MixedAdvancement<24, 24> KRQtoC;
		RandomAdvancement<24, 24> KRCtoP;
		RandomAdvancement<6, 6> finalists;
		array<Player*, 32> wcsQtoC;
		RandomAdvancement<32, 32> wcsCtoP;

		codeb.predict(sim, KRQtoC, frees, rng DEBUGARGS);
		codea.AcceptAdvancement(KRQtoC.advancing_players);
		codea.AcceptPrevBottom24(prev_advs.KR_bottom24, rng);
		codea.predict(sim, KRCtoP, frees, rng DEBUGARGS);
		codes.AcceptAdvancement(KRCtoP.advancing_players);
		codes.AcceptPrevTop8(prev_advs.KR_top8, rng);
		codes.predict(sim, next_advs.KR_top8, next_advs.KR_bottom24, next_advs.gsl_finalists, rng DEBUGARGS);
		for (auto p : next_advs.gsl_finalists) finalists.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);

		quals.predict(sim, frees, wcsQtoC, rng DEBUGARGS);
		chal.AcceptAdvancements(prev_advs.CtoC, wcsQtoC, prev_advs.champ, frees, rng DEBUGARGS);
		chal.predict(sim, frees, next_advs.CtoC, wcsCtoP, rng DEBUGARGS);
		prem.AcceptAdvancements(wcsCtoP DEBUGARGS);
		prem.predict(sim, rng, next_advs.champ DEBUGARGS);

		//spotv.AcceptAdvancement(prev_advs.SPOTV_top4, prev_advs.SPOTV_chalseeds DEBUGARGS);
		//spotv.predict(sim, next_advs.SPOTV_top4, next_advs.SPOTV_chalseeds, rng DEBUGARGS);
	}
};

class Hot6ixCup : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 6> invs;

	Hot6ixCup() : TournamentBase("Hot6ix Cup", 40)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		t_id = 40;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[2] = 125;
		ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;

		array<const char *, 6> sinvs = {
			"skt t1;classic;p;kr", "kt;flash;t;kr", "skt t1;innovation;t;kr", "yoe;parting;p;kr", "skt t1;soo;z;kr", "kt;zest;p;kr"
		};
		for (uint i = 0; i<invs.size(); i++) {
			invs[i] = &SearchPlayer(sinvs[i] DEBUGARGS);
		}
		array<const char*, 16> sro16 = {
			"skt t1;innovation;t;kr", "cj entus;hero;p;kr", "kt;stats;p;kr", "mvp;marineking;t;kr",
			"skt t1;soo;z;kr", "kt;ty;t;kr", "myi;rain;p;kr", "jin air;sos;p;kr",
			"skt t1;classic;p;kr", "kt;flash;t;kr", "cj entus;skyhigh;t;kr", "tcm;soulkey;z;kr",
			"kt;zest;p;kr", "jin air;terminator;p;kr", "cj entus;bbyong;t;kr", "yoe;parting;p;kr"
		};
		for (uint i = 0; i<16; i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2014_Hot6ix_Cup&action=edit&section=7", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaGroupsDH(res, t_id, smatches);
		//smatches[1].players[1]="Rain";//get rid of the damn *
		//smatches[3].players[2]="Parting";
		for (uint g = 0; g<4; g++) {
			auto *m = ro16.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}

		array<const char*, 8> sro8 = {
			/*"mvp;marineking;t;kr", "yoe;parting;p;kr",				"jin air;sos;p;kr", "skt t1;innovation;t;kr",
			";soulkey;z;kr", "kt;zest;p;kr",						"cj entus;bbyong;t;kr", "cj entus;skyhigh;t;kr"
			*/
			"mvp;marineking;t;kr", "yoe;parting;p;kr", "tcm;soulkey;z;kr", "kt;zest;p;kr",
			"jin air;sos;p;kr", "skt t1;innovation;t;kr", "cj entus;bbyong;t;kr", "cj entus;skyhigh;t;kr"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string resb = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2014_Hot6ix_Cup&action=edit&section=12", true, false);
		smatches.clear();
		ScrapeLiquipediaBracketsSimple(resb, t_id, smatches);
		for (uint g = 0; g<4; g++) {
			auto *m = quarterfinals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint g = 4; g<4 + 2; g++) {
			auto *m = semifinals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint g = 4 + 2; g<4 + 2 + 1; g++) {
			auto *m = finals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -125);
			}
		}
		sim.AddPlayerPoints(players[7], 125);//Rain for his forfeit, since he didn't actually lose

		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class KeSPACup1 : public TournamentBase
{
public:
	//Round<40, SwissGroup, 10, StraightGroupAdvancement<20, 20> > qual40;
	//Round<20, SwissGroup, 5, StraightGroupAdvancement<10, 10> > qual20;

	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 8> invs;
	//array<Player*, 8> quals;

	KeSPACup1() : TournamentBase("KeSPA Cup 1", 51)
	{
		t_id = 51;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//qual40.ClearPlacings();
		//qual20.ClearPlacings();
		ro16.ClearPlacings();
		quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		//ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;

		array<const char *, 8> sinvs = {
			"kt;zest;p;kr", "cj entus;hero;p;kr", "jin air;maru;t;kr", "skt t1;dream;t;kr", "kt;life;z;kr", "yoe;parting;p;kr", "mvp;losira;z;kr", "jin air;rogue;z;kr"
		};
		for (uint i = 0; i<invs.size(); i++) {
			invs[i] = &SearchPlayer(sinvs[i] DEBUGARGS);
		}
		array<const char*, 16> sro16 = {
			"kt;zest;p;kr", "dpix;fantasy;t;kr", "mvp;losira;z;kr", "skt t1;dark;z;kr", "kt;life;z;kr", "jin air;terminator;p;kr", "skt t1;dream;t;kr", "kt;stats;p;kr",
			"yoe;parting;p;kr", "tcm;soulkey;z;kr", "cj entus;hero;p;kr", "samsung;dear;p;kr", "jin air;rogue;z;kr", ";myungsik;p;kr", "jin air;maru;t;kr", "cj entus;byul;z;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_KeSPA_Cup_Season_1&action=edit&section=8", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		//if (smatches[6].players[0] == "Rouge") smatches[6].players[0] = "Rogue";
		for (uint i = 0; i < 8; i++) {
			auto m = ro16.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro16.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 8; i < 8+4 && i<smatches.size(); i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8+4; i < 8 + 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8+4+2; i < 8 + 4+2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		for (auto p : invs) {
		player_map[p->player_id] = 1;
		}
		for (auto p : quals) {
		player_map[p->player_id] = 1;
		}
		RandomAdvancement<16, 0> sortro16;
		for (auto p : invs) sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		for (auto p : quals) {
		sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		//sim.AddEvent(197, p->player_id, 1);
		}
		ro16.AcceptAdvancements(sortro16.advancing_players DEBUGARGS);*/

		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -125);
			}
		}

		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class KeSPACup2 : public TournamentBase
{
public:
	//Round<40, SwissGroup, 10, StraightGroupAdvancement<20, 20> > qual40;
	//Round<20, SwissGroup, 5, StraightGroupAdvancement<10, 10> > qual20;

	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 8> invs;
	//array<Player*, 8> quals;

	KeSPACup2() : TournamentBase("KeSPA Cup 2", 58)
	{
		t_id = 58;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//qual40.ClearPlacings();
		//qual20.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		ro16.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;

		array<const char *, 8> sinvs = {
			"cj entus;hero;p;kr", "skt t1;dark;z;kr", "skt t1;classic;p;kr", "skt t1;dream;t;kr", "cj entus;byul;z;kr", "sbenu;curious;z;kr", "jin air;maru;t;kr", "skt t1;soo;z;kr"
		};
		for (uint i = 0; i<invs.size(); i++) {
			invs[i] = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (invs[i]->league == 0) invs[i]->league = t_id;
		}
		/*array<const char *, 8> squals = {
			"yoe;san;p;kr", "jin air;pigbaby;p;kr",
			"kt;flash;t;kr", "dpix;fantasy;t;kr", "sbenu;jjakji;t;kr",
			"skt t1;impact;z;kr", "jin air;rogue;z;kr", "psistm;true;z;kr"
		};
		for (uint i = 0; i<quals.size(); i++) {
			quals[i] = &SearchPlayer(squals[i] DEBUGARGS);
			if (quals[i]->league == 0) quals[i]->league = t_id;
		}*/

		array<const char*, 16> sro16 = {
			"sbenu;curious;z;kr", "kt;flash;t;kr", "skt t1;classic;p;kr", "jin air;rogue;z;kr",
			"skt t1;soo;z;kr", "yoe;san;p;kr", "cj entus;hero;p;kr", "sbenu;jjakji;t;kr",
			"cj entus;byul;z;kr", "jin air;pigbaby;p;kr", "skt t1;dark;z;kr", "dpix;fantasy;t;kr",
			"jin air;maru;t;kr", "skt t1;impact;z;kr",
			"skt t1;dream;t;kr", "psistm;true;z;kr"
		};
		for (uint i = 0; i < 16; i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_KeSPA_Cup_Season_2&action=edit&section=9", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 8; i++) {
			auto m = ro16.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		array<Player*, 2> gsl_finalists = { NULL, NULL };
		predict(sim, rng, gsl_finalists DEBUGARGS);
		for (uint i = 8; i < 8+4 && i<smatches.size(); i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng, gsl_finalists DEBUGARGS);
		for (uint i = 8+4; i < 8 + 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng, gsl_finalists DEBUGARGS);
		for (uint i = 8+4+2; i < 8 + 4+2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng, array<Player*,2> gsl_finalists DEBUGARGSDEF)
	{
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -125);
			}
		}

		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class KeSPACup3 : public TournamentBase
{
public:
	Round<40, SwissGroup, 10, StraightGroupAdvancement<20, 20> > qual40;
	Round<20, SwissGroup, 5, StraightGroupAdvancement<10, 10> > qual20;

	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 2> invs;
	//array<Player*, 8> quals;

	KeSPACup3() : TournamentBase("KeSPA Cup 3", 63)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		qual40.ClearPlacings();
		qual20.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();

		ro16.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;

		array<const char*, 2> sinvs = { "skt t1;soo;z;kr", "skt t1;dark;z;kr" };
		for (uint i = 0; i<sinvs.size(); i++) {
			auto p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}
	}

	void predict(Simulation &sim, RayRand64 &rng, array<Player*, 2> gsl_finalists, array<Player*,2> ssl_finalists DEBUGARGSDEF)
	{
		if (rng.dRand() > 0.25) return;//25% chance to exist

		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		RandomAdvancement<16, 0> sortgroups;
		for (auto p : invs) {
			sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
			player_map[p->player_id] = 1;
		}
		for (auto p : gsl_finalists) {
			sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
			player_map[p->player_id] = 1;
		}
		for (auto p : ssl_finalists) {
			sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
			player_map[p->player_id] = 1;
		}

		RandomAdvancement<40, 0> sortplayers;
		for (uint i = 0; i<40; i++) {
			Player *p = NULL;
			if (i < 20) {
				p = good_koreans[rng.iRand64HQ() % good_koreans.size()];
			}
			else if (i<30) {
				p = good_players[rng.iRand64HQ() % good_players.size()];
			}
			else {
				p = &players[rng.iRand64HQ() % players.size()];
			}
			if (player_map[p->player_id] == 1) {
				i--;
				continue;
			}
			player_map[p->player_id] = 1;
			sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}

		StraightGroupAdvancement<20, 20> qual40_adv;
		StraightGroupAdvancement<10, 10> qual20_adv;
		qual40.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		qual40.predict(sim, t_id, qual40_adv, rng DEBUGARGS);
		qual20.AcceptAdvancements(qual40_adv.advancing_players DEBUGARGS);
		qual20.predict(sim, t_id, qual20_adv, rng DEBUGARGS);
		for (auto p : qual20_adv.advancing_players) sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		array<bool, MAX_PLAYER_ID> player_map2 = { 0 };
		uint subs = 0;
		for (auto &p : sortgroups.advancing_players) {
			if (player_map2[p->player_id]) {
				p = qual20_adv.falling_players[subs];
				subs++;
			}
			player_map2[p->player_id] = 1;
		}

		ro16.AcceptAdvancements(sortgroups.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			for (auto p2 : gsl_finalists) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			for (auto p2 : ssl_finalists) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -125);
			}
		}

		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class IEMOpenBracket : public TournamentBase
{
public:
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > WR1;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > WR2;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > WR3;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > LR1;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > LR2;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > LR3;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > LR4;

	string stime;
	string liquipedia;
	bool readcache;

	IEMOpenBracket(const char*name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
		readcache = false;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, array<Player *, 8> players DEBUGARGSDEF)
	{
		//t_id=12;
		WR1.ClearPlacings();
		WR2.ClearPlacings();
		WR3.ClearPlacings();
		LR1.ClearPlacings();
		LR2.ClearPlacings();
		LR3.ClearPlacings();
		LR4.ClearPlacings();
		WR3.match_placing_to_tournament_placing[0] = 1;
		LR4.match_placing_to_tournament_placing[0] = 1;

		for (uint i = 0; i<8; i++) {
			WR1.matches[i / 2].players[i % 2] = players[i];
		}

		/*for(uint i=0;i<8;i++) {
		uint timestamp=Timestamp(stime);
		//cout << (timestamp-(uint)time(0))/3600 << "\n";
		WR1.matches[i].EasyAddUpcoming(upcoming_matches, timestamp, t_id, WR1.RoundOf());
		}*/

		//cerr << name<<" got res == "<<res.length()<<" bytes\n\n";

		vector<ScrapedGroup> sbrackets;
		if (liquipedia.length()) {
			string res = HttpRequest(liquipedia, readcache, false);
			ScrapeLiquipediaBracketsSimple(res, t_id, sbrackets);
		}
		//cerr << name<< " scraped "<<sbrackets.size()<<" matches\n";
		for (uint g = 0; g<4 && g<sbrackets.size(); g++) {
			if (sbrackets[g].timestamp>0 && stime.length()) DEBUGOUTSTACK(ERR, stime, "overriding timestamp with stime!");
			if (stime.length()) sbrackets[g].timestamp = Timestamp(stime);
			//sbrackets[g].timestamp==Timestamp(stime);
			//sbrackets[g].timestamp-=14*3600;// <<<<==================================================
			auto *match = WR1.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, WR1.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		RayRand64 rng;
		Simulation sim;
		StraightAdvancement<2, 2> ToGroups;
		predict(sim, rng, ToGroups DEBUGARGS);

		for (uint g = 4; g<4 + 2 && g<sbrackets.size(); g++) {
			auto *match = WR2.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, WR2.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		predict(sim, rng, ToGroups DEBUGARGS);

		for (uint g = 4 + 2; g<4 + 2 + 1 && g<sbrackets.size(); g++) {
			auto *match = WR3.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, WR3.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		predict(sim, rng, ToGroups DEBUGARGS);

		for (uint g = 7; g<7 + 2 && g<sbrackets.size(); g++) {
			auto *match = LR1.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, LR1.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		predict(sim, rng, ToGroups DEBUGARGS);

		for (uint g = 7 + 2; g<7 + 2 + 2 && g<sbrackets.size(); g++) {
			auto *match = LR2.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, LR2.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		predict(sim, rng, ToGroups DEBUGARGS);

		for (uint g = 7 + 2 + 2; g<7 + 2 + 2 + 1 && g<sbrackets.size(); g++) {
			auto *match = LR3.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, LR3.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}

		predict(sim, rng, ToGroups DEBUGARGS);

		//cerr << LR4.matches[0].players[0]->name << " vs "<<LR4.matches[0].players[1]->name<<"\n";
		for (uint g = 7 + 2 + 2 + 1; g<7 + 2 + 2 + 1 + 1 && g<sbrackets.size(); g++) {
			auto *match = LR4.GetMatch(sbrackets[g].players);
			if (match) {
				match->ReadOldResults(sbrackets[g], prev_matches, upcoming_matches, LR4.RoundOf());
			}
			else {
				cerr << sbrackets[g].players[0] << " vs " << sbrackets[g].players[1] << "\n";
			}
			assert(match != NULL);
		}
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		WR1.ClearPlacings();
		WR2.ClearPlacings();
		WR3.ClearPlacings();
		LR1.ClearPlacings();
		LR2.ClearPlacings();
		LR3.ClearPlacings();
		LR4.ClearPlacings();
		WR3.match_placing_to_tournament_placing[0] = 1;
		LR4.match_placing_to_tournament_placing[0] = 1;
	}

	void predict(Simulation &sim, RayRand64 &rng, StraightAdvancement<2, 2> &ToGroups DEBUGARGSDEF)
	{
		if (completion == 255) return;

		StraightAdvancement<4, 4> WR1adv;
		StraightAdvancement<2, 2> WR2adv;
		StraightAdvancement<1, 1> WR3adv;
		StraightAdvancement<2, 2> LR1adv;
		StraightAdvancement<2, 2> LR2adv;
		StraightAdvancement<1, 1> LR3adv;
		StraightAdvancement<1, 1> LR4adv;

		WR1.predict(sim, t_id, WR1adv, rng DEBUGARGS);
		WR2.AcceptAdvancements(WR1adv.advancing_players DEBUGARGS);
		WR2.predict(sim, t_id, WR2adv, rng DEBUGARGS);
		WR3.AcceptAdvancements(WR2adv.advancing_players DEBUGARGS);
		WR3.predict(sim, t_id, WR3adv, rng DEBUGARGS);
		//ToGroups.AddAdvancingPlayer(WR3adv.advancing_players[0], 1,0,0,0,0,rng);
		ToGroups.advancing_players[0] = WR3adv.advancing_players[0];

		LR1.AcceptAdvancements(WR1adv.falling_players DEBUGARGS);
		LR1.predict(sim, t_id, LR1adv, rng DEBUGARGS);
		LR2.matches[0].players[0] = WR2adv.falling_players[1];
		LR2.matches[1].players[0] = WR2adv.falling_players[0];
		LR2.matches[0].players[1] = LR1adv.advancing_players[0];
		LR2.matches[1].players[1] = LR1adv.advancing_players[1];
		LR2.predict(sim, t_id, LR2adv, rng DEBUGARGS);
		LR3.AcceptAdvancements(LR2adv.advancing_players DEBUGARGS);
		LR3.predict(sim, t_id, LR3adv, rng DEBUGARGS);
		LR4.matches[0].players[0] = WR3adv.falling_players[0];
		LR4.matches[0].players[1] = LR3adv.advancing_players[0];
		LR4.predict(sim, t_id, LR4adv, rng DEBUGARGS);

		//ToGroups.AddAdvancingPlayer(LR4adv.advancing_players[0], 2,1,0,0,0,rng);
		ToGroups.advancing_players[1] = LR4adv.advancing_players[0];
		ToGroups.falling_players[0] = LR4adv.falling_players[0];
	}
};

class IEMSanJose : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	IEMOpenBracket ob1;
	IEMOpenBracket ob2;

	array<Player*, 4> invs;
	//array<Player*,12> confs;
	array<Player*, 8> openconfs1;
	array<Player*, 8> openconfs2;

	IEMSanJose() : TournamentBase("IEM San Jose", 42), ob1("IEM San Jose Stage 1", 41), ob2("IEM San Jose Stage 1", 41)
	{
		t_id = 42;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ob2.stime=ob1.stime="August 28, 2014 15:00 {{Abbr/CDT";
		//ob2.stime="July 16, 2014 07:00 {{Abbr/CDT";
		ob1.readcache = true;
		ob2.readcache = true;
		ob1.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_San_Jose/Stage_1&action=edit&section=T-1";
		ob2.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_San_Jose/Stage_1&action=edit&section=T-2";

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[2] = 125;
		ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 5;

		array<const char*, 16> sconfs = {
			"myi;rain;p;kr", "eg;huk;p;ca", "psistm;true;z;kr", "liquid;snute;z;no",
			";first;p;kr", ";revival;z;kr", "cm storm;polt;t;kr", "eg;jaedong;z;kr",
			"millenium;forgg;t;kr", "jin air;symbol;z;kr", "mvp;gumiho;t;kr", "roccat;hyun;z;kr",
			"cj entus;hero;p;kr", "kt;super;p;kr", "col;qxc;t;us", "SBENU;bomber;t;kr"
		};
		for (uint i = 0; i<16; i++) {
			Player *p = &SearchPlayer(sconfs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 4> sinvs = {
			"SBENU;bomber;t;kr", "liquid;snute;z;no", "eg;jaedong;z;kr", "roccat;hyun;z;kr"
		};
		for (uint i = 0; i<4; i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}

		array<const char*, 8> sopenconfs1 = {
			"psistm;intense;t;us", "root;caliber;r;us", "property;zanster;z;se", "kt;super;p;kr",
			"f3;puck;p;us", ";walkover", "mvp;gumiho;t;kr", "eg;huk;p;ca"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(sopenconfs1[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			openconfs1[i] = p;
		}
		array<const char*, 8> sopenconfs2 = {
			"psistm;true;z;kr", ";top;t;kr", "jin air;symbol;z;kr", "invasion;harstem;p;nl",
			"mvp;seed;p;kr", "root;catz;z;pe", "psistm;phog;z;us", ";revival;z;kr"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(sopenconfs2[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			openconfs2[i] = p;
		}

		ob1.init(prev_matches, upcoming_matches, openconfs1 DEBUGARGS);
		ob2.init(prev_matches, upcoming_matches, openconfs2 DEBUGARGS);

		vector<string> urls;
		for (uint g = 1; g <= 4; g++) {
			string url = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_San_Jose/Stage_2&action=edit&section=T-" + ToString(g);
			urls.push_back(url);
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint g = 0; g<4; g++) {
			string res = ress[g];
			vector<ScrapedGroup> sgroup;
			ScrapeLiquipediaGroupsDH(res, t_id, sgroup);
			//sgroup[0].timestamp=Timestamp("December 4, 2014");
			auto *m = ro16.GetMatch(sgroup[0].players);
			if (m) m->ReadOldResults(sgroup[0], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}

		array<const char*, 8> squarters = {
			"cj entus;hero;p;kr", "liquid;snute;z;no", "eg;jaedong;z;kr", "mvp;gumiho;t;kr",
			"myi;rain;p;kr", "millenium;forgg;t;kr", "SBENU;bomber;t;kr", ";revival;z;kr"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(squarters[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_San_Jose/Stage_3&action=edit", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint g = 0; g<4; g++) {
			auto *m = quarterfinals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint g = 4; g<4 + 2; g++) {
			auto *m = semifinals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint g = 4 + 2; g<4 + 2 + 1; g++) {
			smatches[g].timestamp = smatches[g - 1].timestamp + 3600;
			auto *m = finals.GetMatch(smatches[g].players);
			if (m) m->ReadOldResults(smatches[g], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//return;
		StraightAdvancement<2, 2> O1togroups;
		StraightAdvancement<2, 2> O2togroups;

		ob1.predict(sim, rng, O1togroups DEBUGARGS);
		ob2.predict(sim, rng, O2togroups DEBUGARGS);

		//RandomAdvancement<8,8> ro16adv;
		StraightGroupAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		/*RandomAdvancement<4,0> openadv;
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[1], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[1], 1,1,1,1,1,rng);

		ro16.matches[0].players[3]=openadv.advancing_players[0];
		ro16.matches[1].players[3]=openadv.advancing_players[1];
		ro16.matches[2].players[3]=openadv.advancing_players[2];
		ro16.matches[3].players[3]=openadv.advancing_players[3];*/

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p1, -125);
					break;
				}
			}
		}

		//[a1,d2,b1,c2, c1,b2,d1,a2]
		/*quarterfinals.matches[0].players[0]=ro16adv.advancing_players[6];
		quarterfinals.matches[0].players[1]=ro16adv.advancing_players[3];
		quarterfinals.matches[1].players[0]=ro16adv.advancing_players[7];
		quarterfinals.matches[1].players[1]=ro16adv.advancing_players[2];
		quarterfinals.matches[2].players[0]=ro16adv.advancing_players[0];
		quarterfinals.matches[2].players[1]=ro16adv.advancing_players[5];
		quarterfinals.matches[3].players[0]=ro16adv.advancing_players[1];
		quarterfinals.matches[3].players[1]=ro16adv.advancing_players[4];*/
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class IEMTaipei : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	IEMOpenBracket ob1;
	IEMOpenBracket ob2;

	array<Player*, 4> invs;
	//array<Player*,12> confs;
	//array<Player*, 8> openconfs1;
	//array<Player*, 8> openconfs2;

	IEMTaipei() : TournamentBase("IEM Taipei", 44), ob1("IEM Taipei Stage 1", 43), ob2("IEM Taipei Stage 1", 43)
	{
		t_id = 44;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ob2.stime=ob1.stime="January 28, 2015 07:00 {{Abbr /CDT";
		//ob2.stime="July 16, 2014 07:00 {{Abbr/CDT";
		ob1.readcache = true;
		ob2.readcache = true;
		ob1.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_Taipei/Stage_1&action=edit&section=T-1";
		ob2.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_Taipei/Stage_1&action=edit&section=T-2";

		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[2] = 125;
		ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		array<const char*, 16> sconfs = {
			"cm storm;polt;t;kr", "cj entus;hero;p;kr", "hka;sen;z;tw", "roccat;hyun;z;kr",
			"invasion;harstem;p;nl", "myi;rain;p;kr", "skt t1;classic;p;kr", "jin air;check;z;kr",
			"millenium;forgg;t;kr", "kt;life;z;kr", "yoe;san;p;kr", "tcm;soulkey;z;kr",
			"root;hydra;z;kr", "yoe;parting;p;kr", "jin air;maru;t;kr", "psistm;true;z;kr"
		};
		for (uint i = 0; i<sconfs.size(); i++) {
			Player *p = &SearchPlayer(sconfs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			//confs[i] = p;
			ro16.matches[i / 4].players[i % 4] = p;
			//if (i>=8) invs[i - 8] = p;
		}
		array<const char*, 4> sinvs = { "cj entus;hero;p;kr", "yoe;parting;p;kr", "kt;life;z;kr", "myi;rain;p;kr" };
		for (uint i = 0; i<sinvs.size(); i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}

		array<const char*, 16> sopens = {
			"sbenu;jjakji;t;kr", "skt t1;impact;z;kr", "myi;sacsri;z;kr", "sbenu;leenock;z;kr", "psistm;true;z;kr", "tcm;soulkey;z;kr", "yoe;has;p;tw", "liquid;hero;p;kr",
			"roccat;hyun;z;kr", "jin air;check;z;kr", ";revival;z;kr", "SBENU;dongraegu;z;kr", "dpix;patience;p;kr", "yoe;ian;z;tw", "SBENU;daisy;p;kr", "liquid;snute;z;no"
		};
		array<Player*, 8> opens1;
		array<Player*, 8> opens2;
		for (uint i = 0; i<sopens.size(); i++) {
			Player *p = &SearchPlayer(sopens[i] DEBUGARGS);
			if (p->league == 0) p->league = ob1.t_id;
			if (i < 8) opens1[i] = p;
			else opens2[i - 8] = p;
		}
		ob1.init(prev_matches, upcoming_matches, opens1 DEBUGARGS);
		ob2.init(prev_matches, upcoming_matches, opens2 DEBUGARGS);

		array<const char*, 8> sro8 = {
			"yoe;parting;p;kr", "skt t1;classic;p;kr",
			"roccat;hyun;z;kr", "kt;life;z;kr",
			"cj entus;hero;p;kr", "jin air;maru;t;kr",
			"tcm;soulkey;z;kr", "myi;rain;p;kr",
		};
		for (uint i = 0; i<sro8.size(); i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			//confs[i] = p;
			quarterfinals.matches[i / 2].players[i % 2] = p;
			//if (i>=8) invs[i - 8] = p;
		}

		vector<string> urls;
		for (uint g = 1; g <= 4; g++) {
			string url = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_Taipei/Stage_2&action=edit&section=T-" + ToString(g);
			urls.push_back(url);
		}
		vector<string> ress = HttpRequests(urls, true, false);
		for (uint i = 0; i < 4; i++) {
			vector<ScrapedGroup> sgroup;
			ScrapeLiquipediaGroupsDH(ress[i], t_id, sgroup);
			//sgroup[0].timestamp = Timestamp("January 29, 2015 07:00 {{Abbr /CDT");
			auto *m = ro16.GetMatch(sgroup[0].players);
			if (m) m->ReadOldResults(sgroup[0], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_Taipei&action=edit&section=10", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto *m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4 + 2; i++) {
			auto *m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1; i++) {
			auto *m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//return;
		/*RandomAdvancement<12, 12> sortgroups;
		for (auto c : confs) {
		sortgroups.AddAdvancingPlayer(c, 1, 1, 1, 1, 1, rng);
		}*/
		/*for (uint i = 0; i<12-confs.size(); i++) {
		Player *p = NULL;
		if (i<3) {
		p = good_players[rng.iRand64HQ() % good_players.size()];
		}
		else {
		while (p == NULL || p->player_id == 0) {
		p = &players[rng.iRand64HQ() % players.size()];
		}
		}
		bool found = false;
		for (auto a : sortgroups.advancing_players) {
		if (a == p) {
		found = true;
		break;
		}
		}
		if (found) {
		i--;
		continue;
		}
		sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/

		/*RandomAdvancement<16, 16> sortplayers;
		for (uint i = 0; i<16; i++) {
		Player *p = NULL;
		if (i<10) {
		p = good_players[rng.iRand64HQ() % good_players.size()];
		}
		else {
		while (p == NULL || p->player_id == 0) {
		p = &players[rng.iRand64HQ() % players.size()];
		}
		}
		bool found = false;
		for (auto a : sortplayers.advancing_players) {
		if (a == p) {
		found = true;
		break;
		}
		}
		for (auto a : sortgroups.advancing_players) {
		if (a == p) {
		found = true;
		break;
		}
		}
		if (found) {
		i--;
		continue;
		}
		sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}

		for (uint i = 0; i < 8; i++) {
		ob1.WR1.matches[i / 2].players[i % 2] = sortplayers.advancing_players[i];
		}
		for (uint i = 0; i < 8; i++) {
		ob2.WR1.matches[i / 2].players[i % 2] = sortplayers.advancing_players[i+8];
		}*/

		/*StraightAdvancement<2, 2> O1togroups;
		StraightAdvancement<2, 2> O2togroups;

		ob1.predict(sim, rng, O1togroups DEBUGARGS);
		ob2.predict(sim, rng, O2togroups DEBUGARGS);

		//RandomAdvancement<8,8> ro16adv;
		RandomAdvancement<4,0> openadv;
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[1], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[1], 1,1,1,1,1,rng);

		ro16.matches[0].players[3]=openadv.advancing_players[0];
		ro16.matches[1].players[3]=openadv.advancing_players[1];
		ro16.matches[2].players[3]=openadv.advancing_players[2];
		ro16.matches[3].players[3]=openadv.advancing_players[3];*/

		/*for (uint i = 0; i < 12; i++) {
		ro16.matches[i / 3].players[i % 3] = sortgroups.advancing_players[i];
		}*/
		StraightGroupAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p1, -125);
					break;
				}
			}
		}

		//[a1,d2,b1,c2, c1,b2,d1,a2]
		/*quarterfinals.matches[0].players[0]=ro16adv.advancing_players[6];
		quarterfinals.matches[0].players[1]=ro16adv.advancing_players[3];
		quarterfinals.matches[1].players[0]=ro16adv.advancing_players[7];
		quarterfinals.matches[1].players[1]=ro16adv.advancing_players[2];
		quarterfinals.matches[2].players[0]=ro16adv.advancing_players[0];
		quarterfinals.matches[2].players[1]=ro16adv.advancing_players[5];
		quarterfinals.matches[3].players[0]=ro16adv.advancing_players[1];
		quarterfinals.matches[3].players[1]=ro16adv.advancing_players[4];*/
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class IEMKatowice : public TournamentBase
{
public:
	//Round<32, RoundRobin<4,1>, 8, RandomAdvancement<8,24> > quals;
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 8> invs;
	//array<Player*, 8> quals;

	IEMKatowice() : TournamentBase("IEM Katowice", 45)
	{
		t_id = 45;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//quals.ClearPlacings();
		ro16.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;
		ro16.points_for_placing[1] = 250;
		quarterfinals.points_for_placing[1] = 400;
		semifinals.points_for_placing[1] = 550;
		finals.points_for_placing[1] = 800;
		finals.points_for_placing[0] = 1500;
		finals.match_placing_to_tournament_placing[0] = 1;

		array<const char *, 8> sinvites = {
			"liquid;taeja;t;kr", "samsung;solar;z;kr", "kt;zest;p;kr", "kt;flash;t;kr", "cj entus;hero;p;kr", "myi;rain;p;kr", "jin air;maru;t;kr", "kt;life;z;kr"
		};
		for (uint i = 0; i<8; i++) {
			Player *p = &SearchPlayer(sinvites[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}
		/*array<const char *, 8> squals = {
		"dpix;patience;p;kr", "skt t1;innovation;t;kr", "skt t1;dark;z;kr",
		"jin air;trap;p;kr", "cj entus;bbyong;t;kr",
		"dpix;fantasy;t;kr", "root;hydra;z;kr", "jin air;cure;t;kr"
		};
		for (uint i = 0; i<8; i++) {
		Player *p = &SearchPlayer(squals[i] DEBUGARGS);
		if (p->league == 0) p->league = t_id;
		quals[i] = p;
		}*/
		array<const char*, 16> sro16 = {
			"cj entus;hero;p;kr", "jin air;cure;t;kr", "cj entus;bbyong;t;kr", "samsung;solar;z;kr", "kt;zest;p;kr", "root;hydra;z;kr", "skt t1;innovation;t;kr", "kt;life;z;kr",
			"kt;flash;t;kr", "jin air;trap;p;kr", "dpix;fantasy;t;kr", "myi;rain;p;kr", "jin air;maru;t;kr", "dpix;patience;p;kr", "skt t1;dark;z;kr", "liquid;taeja;t;kr"
		};
		for (uint i = 0; i<16; i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_IX_-_World_Championship&action=edit&section=8", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 8; i++) {
			auto *m = ro16.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 8; i < 8 + 4; i++) {
			auto *m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8 + 4; i < 8 + 4 + 2; i++) {
			auto *m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8 + 4 + 2; i < 8 + 4 + 2 + 1; i++) {
			auto *m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		StraightAdvancement<8, 8> Ro16toRo8;
		StraightAdvancement<4, 4> Ro8toRo4;
		StraightAdvancement<2, 2> Ro4toRo2;
		StraightAdvancement<1, 1> finalsadv;
		//simulate qualifiers
		/*RandomAdvancement<32, 0> sortgroups;
		for (uint i = 0; i < 32; i++) {
		Player *p = NULL;
		if (i < 20) {
		p = good_players[rng.iRand64HQ() % good_players.size()];
		}
		else {
		while (p == NULL || p->player_id == 0) {
		p = &players[rng.iRand64HQ() % players.size()];
		}
		}
		bool found = false;
		for (auto a : sortgroups.advancing_players) {
		if (a == p) {
		found = true;
		break;
		}
		}
		for (auto a : invs) {
		if (a == p) {
		found = true;
		break;
		}
		}
		if (found) {
		i--;
		continue;
		}
		sortgroups.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		RandomAdvancement<8, 24> advquals;
		quals.AcceptAdvancements(sortgroups.advancing_players DEBUGARGS);
		quals.predict(sim, t_id, advquals, rng DEBUGARGS);*/

		/*RandomAdvancement<8, 0> sortinvs;
		for (auto p : invs) {
		sortinvs.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		RandomAdvancement<8, 0> sortquals;
		for (auto p : quals) {
		sortquals.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		for (uint i = 0; i < 8; i++) {
		ro16.matches[i].players[0] = sortinvs.advancing_players[i];
		ro16.matches[i].players[1] = sortquals.advancing_players[i];
		}*/
		ro16.predict(sim, t_id, Ro16toRo8, rng DEBUGARGS);
		for (auto p1 : Ro16toRo8.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p1, -250);
					break;
				}
			}
		}

		quarterfinals.AcceptAdvancements(Ro16toRo8.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, Ro8toRo4, rng DEBUGARGS);
		semifinals.AcceptAdvancements(Ro8toRo4.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, Ro4toRo2, rng DEBUGARGS);
		finals.AcceptAdvancements(Ro4toRo2.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class IEMShenzhen : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	//IEMOpenBracket ob1;
	//IEMOpenBracket ob2;

	array<Player*, 9> invs;
	//array<Player*,12> confs;
	//array<Player*, 8> openconfs1;
	//array<Player*, 8> openconfs2;

	IEMShenzhen() : TournamentBase("IEM Shenzhen", 44)//, ob1("IEM Shenzhen Stage 1", 57), ob2("IEM Shenzhen Stage 1", 57)
	{
		t_id = 56;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[2] = 125;
		ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		array<const char*, 9> sinvs = { "yoe;parting;p;kr", "myi;rain;p;kr", "ig;jim;p;cn", "axiom;heart;t;kr",
			"root;supernova;t;kr", "mvp;losira;z;kr", "mvp;yonghwa;p;kr", "bheart;cyan;p;cn", ";bistork;p;hk" };
		for (uint i = 0; i<sinvs.size(); i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}
		array<const char*, 16> sro16 = {
			"kt;ty;t;kr", "mvp;losira;z;kr", "root;supernova;t;kr", "axiom;heart;t;kr",
			"skt t1;classic;p;kr", "mvp;yonghwa;p;kr", "liquid;snute;z;no", "myi;rain;p;kr",
			"cj entus;hero;p;kr", "bheart;cyan;p;cn", "at;uthermal;t;nl", "ig;jim;p;cn",
			"ig;xy;t;cn", ";bistork;p;hk", "root;kelazhur;t;br", "yoe;parting;p;kr"
			/* */
		};
		for (uint i = 0; i<sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			//confs[i] = p;
			//ro16.matches[i / 3].players[i % 3] = p;
			ro16.matches[i / 4].players[i % 4] = p;
		}
		/*ro16.matches[0].players[0] = NULL;
		array<Player*, 8> ob1players, ob2players;
		array<const char*, 8> obplayers = {
			"mvp;seed;p;kr", "root;supernova;t;kr", "axiom;alicia;p;kr", ";saber;p;cn",
			"mvp;yonghwa;p;kr", "bheart;cyan;p;cn", "mvp;losira;z;kr", ";bistork;p;hk"
		};
		for (uint i = 0; i < obplayers.size(); i++) {
			auto p = &SearchPlayer(obplayers[i] DEBUGARGS);
			if (p->league == 0) p->league = ob1.t_id;
			if (i < 4) {
				ob1players[i * 2] = p;
				ob1players[i * 2 + 1] = &players[0];
			}
			else {
				ob2players[(i-4) * 2] = p;
				ob2players[(i-4) * 2 + 1] = &players[0];
			}
		}
		
		ob1.init(prev_matches, upcoming_matches, ob1players DEBUGARGS);
		ob2.init(prev_matches, upcoming_matches, ob2players DEBUGARGS);*/

		array<const char*, 8> sro8 = {
			"cj entus;hero;p;kr", "ig;xy;t;cn", "skt t1;classic;p;kr", "mvp;losira;z;kr",
			"yoe;parting;p;kr", "at;uthermal;t;nl", "kt;ty;t;kr", "liquid;snute;z;no"
			/* "root;supernova;t;kr", "axiom;heart;t;kr",
			 "mvp;yonghwa;p;kr",  "myi;rain;p;kr",
			 "bheart;cyan;p;cn", "ig;jim;p;cn",
			 ";bistork;p;hk", "root;kelazhur;t;br",*/
		};
		for (uint i = 0; i<sro8.size(); i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_Shenzhen/Stage_2&action=edit", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (auto &g : sgroups) {
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_Shenzhen&action=edit&section=10", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*StraightAdvancement<2, 2> O1togroups;
		StraightAdvancement<2, 2> O2togroups;

		ob1.predict(sim, rng, O1togroups DEBUGARGS);
		ob2.predict(sim, rng, O2togroups DEBUGARGS);

		//RandomAdvancement<8,8> ro16adv;
		RandomAdvancement<4,0> openadv;
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[1], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[0], 1,1,1,1,1,rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[1], 1,1,1,1,1,rng);

		ro16.matches[0].players[3]=openadv.advancing_players[0];
		ro16.matches[1].players[3]=openadv.advancing_players[1];
		ro16.matches[2].players[3]=openadv.advancing_players[2];
		ro16.matches[3].players[3]=openadv.advancing_players[3];

		Player* fifth = &players[0];
		if (rng.dRand() < 0.5) {
			fifth = ob1.LR4.matches[0].players[0];
			if (fifth == O1togroups.advancing_players[0] || fifth == O1togroups.advancing_players[1]) fifth = ob1.LR4.matches[0].players[1];
		}
		else {
			fifth = ob2.LR4.matches[0].players[0];
			if (fifth == O2togroups.advancing_players[0] || fifth == O2togroups.advancing_players[1]) fifth = ob2.LR4.matches[0].players[1];
		}
		if (fifth != &players[0]) sim.AddEvent(ob1.t_id, fifth->player_id, 1);
		ro16.matches[0].players[0] = fifth;*/

		StraightGroupAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p1, -125);
					break;
				}
			}
		}

		//[a1,d2,b1,c2, c1,b2,d1,a2]
		/*quarterfinals.matches[0].players[0]=ro16adv.advancing_players[6];
		quarterfinals.matches[0].players[1]=ro16adv.advancing_players[3];
		quarterfinals.matches[1].players[0]=ro16adv.advancing_players[7];
		quarterfinals.matches[1].players[1]=ro16adv.advancing_players[2];
		quarterfinals.matches[2].players[0]=ro16adv.advancing_players[0];
		quarterfinals.matches[2].players[1]=ro16adv.advancing_players[5];
		quarterfinals.matches[3].players[0]=ro16adv.advancing_players[1];
		quarterfinals.matches[3].players[1]=ro16adv.advancing_players[4];*/
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class IEMGamescom : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, StraightGroupAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	//IEMOpenBracket ob1;
	//IEMOpenBracket ob2;

	array<Player*, 4> invs;
	//array<Player*, 12> confs;
	array<Player*, 8> openconfs1;
	array<Player*, 8> openconfs2;

	IEMGamescom() : TournamentBase("IEM Gamescom", 60)//, ob1("IEM Gamescom Stage 1", 62), ob2("IEM Gamescom Stage 1", 62)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[2] = 125;
		ro16.points_for_placing[3] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;

		array<const char*, 4> sinvs = { "myi;rain;p;kr", "acer;mma;t;kr", "dpix;fantasy;t;kr", "millenium;forgg;t;kr" };
		for (uint i = 0; i < sinvs.size(); i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}
		/*array<const char*, 12> sro16 = {
			"skt t1;soo;z;kr", ";happy;t;ru", "acer;mma;t;kr",
			"skt t1;innovation;t;kr", "millenium;lilbow;p;fr", "dpix;fantasy;t;kr",
			";state;p;us", "invasion;lambo;z;de", "millenium;forgg;t;kr",
			";stardust;p;kr", "acer;nerchio;z;pl", ";walkover"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			//confs[i] = p;
			ro16.matches[i / 3].players[i % 3] = p;
		}
		ro16.matches[3].players[2] = NULL;*/
		array<const char*, 16> sro16 = {
			"skt t1;soo;z;kr", "mouz;heromarine;t;de", ";happy;t;ru", "acer;mma;t;kr",
			"skt t1;innovation;t;kr", "liquid;ret;z;nl", "millenium;lilbow;p;fr", "dpix;fantasy;t;kr",
			";state;p;us", "at;uthermal;t;nl", "invasion;lambo;z;de", "millenium;forgg;t;kr",
			";stardust;p;kr", "dpix;patience;p;kr", "acer;nerchio;z;pl", "liquid;tlo;z;de"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}

		array<const char*, 16> sopens = {
			"dpix;patience;p;kr", "karnage;inzane;t;de", "isimba;verdi;p;ru", "acer;bly;z;ua",
			"mouz;heromarine;t;de", ";tarrantius;p;de", ";blackcat;z;es", "liquid;mana;p;pl",
			"liquid;tlo;z;de", ";cpu;p;de", "karnage;jade;p;pl", "myi;ptitdrogo;p;fr",
			"at;uthermal;t;nl", "esc;goody;t;de", "tsk;ziggy;t;pl", "liquid;ret;z;nl"
		};
		for (uint i = 0; i < sopens.size(); i++) {
			Player *p = &SearchPlayer(sopens[i] DEBUGARGS);
			//if (p->league == 0) p->league = ob1.t_id;
			if (i < 8) openconfs1[i] = p;
			else openconfs2[i - 8] = p;
		}

		array<const char*, 8> sro8 = {
			"skt t1;soo;z;kr", "invasion;lambo;z;de", "dpix;fantasy;t;kr", "dpix;patience;p;kr",
			"acer;nerchio;z;pl", "acer;mma;t;kr", "millenium;forgg;t;kr", "skt t1;innovation;t;kr"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		/*ob1.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_gamescom/Stage_1&action=edit&section=T-1";
		ob1.stime = "August 5, 2015 5:00 {{Abbr /CDT";
		ob2.liquipedia = "http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_gamescom/Stage_1&action=edit&section=T-2";
		ob2.stime = "August 5, 2015 5:00 {{Abbr /CDT";
		ob1.init(prev_matches, upcoming_matches, openconfs1 DEBUGARGS);
		ob2.init(prev_matches, upcoming_matches, openconfs2 DEBUGARGS);*/

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_gamescom/Stage_2&action=edit", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (auto &g : sgroups) {
			//if (g.timestamp == 0) g.timestamp = Timestamp("August 6, 2015 10:00 {{Abbr /CDT");
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=IEM_Season_X_-_gamescom&action=edit&section=10", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4;i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4+2; i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*StraightAdvancement<2, 2> O1togroups;
		StraightAdvancement<2, 2> O2togroups;

		ob1.predict(sim, rng, O1togroups DEBUGARGS);
		ob2.predict(sim, rng, O2togroups DEBUGARGS);

		RandomAdvancement<4, 0> openadv;
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[0], 1, 1, 1, 1, 1, rng);
		openadv.AddAdvancingPlayer(O1togroups.advancing_players[1], 1, 1, 1, 1, 1, rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[0], 1, 1, 1, 1, 1, rng);
		openadv.AddAdvancingPlayer(O2togroups.advancing_players[1], 1, 1, 1, 1, 1, rng);

		ro16.matches[0].players[3] = openadv.advancing_players[0];
		ro16.matches[1].players[3] = openadv.advancing_players[1];
		ro16.matches[2].players[3] = openadv.advancing_players[2];
		ro16.matches[3].players[3] = openadv.advancing_players[3];
		if (rng() < 0.5) {
			ro16.matches[3].players[2] = O1togroups.falling_players[0];
		}
		else {
			ro16.matches[3].players[2] = O2togroups.falling_players[0];
		}*/

		StraightGroupAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p1, -125);
					break;
				}
			}
		}

		//[a1,d2,b1,c2, c1,b2,d1,a2]
		/*quarterfinals.matches[0].players[0] = ro16adv.advancing_players[6];
		quarterfinals.matches[0].players[1] = ro16adv.advancing_players[3];
		quarterfinals.matches[1].players[0] = ro16adv.advancing_players[7];
		quarterfinals.matches[1].players[1] = ro16adv.advancing_players[2];
		quarterfinals.matches[2].players[0] = ro16adv.advancing_players[0];
		quarterfinals.matches[2].players[1] = ro16adv.advancing_players[5];
		quarterfinals.matches[3].players[0] = ro16adv.advancing_players[1];
		quarterfinals.matches[3].players[1] = ro16adv.advancing_players[4];*/
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class CopenhagenSpring : public TournamentBase
{
public:
	Round<32, SwissGroup, 8, RandomAdvancement<16, 16> > ro32;
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	//array<Player*, 21> confs;

	CopenhagenSpring() : TournamentBase("Copenhagen Spring", 49)
	{
		t_id = 49;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		//finals.match_placing_to_tournament_placing[1] = 0;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[3] = ro16.points_for_placing[2] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;
		semifinals.best_of = 5;
		finals.best_of = 7;

		/*array<const char*, 21> sconfs = {
		";elfi;p;fi", "property;naniwa;p;se", ";spacemarine;p;dk", "dpix;patience;p;kr", "yoe;parting;p;kr", "myi;rain;p;kr", "ecv;snovski;p;dk",
		"property;morrow;t;se", "dpix;fantasy;t;kr", "axiom;ryung;t;kr", "at;uthermal;t;nl", "rblood;lillekanin;t;dk", "root;supernova;t;kr",
		"liquid;ret;z;nl", "invasion;miniraser;z;se", ";revolver;z;ru", ";targa;z;no", "property;zanster;z;se", "karnage;namshar;z;se", "eg;jaedong;z;kr", "menace;zhugeliang;z;fi"
		};
		for (uint i = 0; i < sconfs.size(); i++) {
		Player *p = &SearchPlayer(sconfs[i] DEBUGARGS);
		if (p->league == 0) p->league = t_id;
		confs[i] = p;
		}*/
		array<const char*, 32> sro32 = {
			"yoe;parting;p;kr", "rblood;pappijoe;p;dk", "ecv;spazymazy;z;dk", "axiom;ryung;t;kr",
			"dpix;fantasy;t;kr", "ecv;pox;p;dk", "rblood;lillekanin;t;dk", "eg;jaedong;z;kr",
			"myi;rain;p;kr", ";utopi;p;dk", "dpix;patience;p;kr", "invasion;miniraser;z;se",
			";elfi;p;fi", ";walkover", "ecv;snovski;p;dk", ";targa;z;no",
			"root;supernova;t;kr", ";callmesnacks;p;dk", "menace;zhugeliang;z;fi", "darkstar;femo;z;se",
			"liquid;ret;z;nl", ";paingamer;p;dk", "karnage;namshar;z;se", ";spacemarine;p;dk",
			"property;zanster;z;se", ";wrath;z;dk", ";walkover", "karnage;strintern;p;dk",
			"at;uthermal;t;nl", ";danner;z;dk", "property;morrow;t;se", ";raggy;z;dk"
		};
		for (uint i = 0; i < sro32.size(); i++) {
			Player *p = &SearchPlayer(sro32[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 16> sro16 = {
			"yoe;parting;p;kr", ";targa;z;no", "root;supernova;t;kr", "at;uthermal;t;nl",
			"dpix;fantasy;t;kr", "invasion;miniraser;z;se", "liquid;ret;z;nl", "karnage;strintern;p;dk",
			"myi;rain;p;kr", "eg;jaedong;z;kr", "property;zanster;z;se", ";spacemarine;p;dk",
			";elfi;p;fi", "axiom;ryung;t;kr", "property;morrow;t;se", "menace;zhugeliang;z;fi",
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 8> sro8 = {
			"yoe;parting;p;kr", ";elfi;p;fi",
			"axiom;ryung;t;kr", "root;supernova;t;kr",
			"dpix;fantasy;t;kr", "eg;jaedong;z;kr",
			"myi;rain;p;kr", "invasion;miniraser;z;se"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Copenhagen_Games_Spring_2015/Group_Stage_2&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro32;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro32);
		for (uint i = 0; i < 8; i++) {
			auto m = ro32.GetMatch(sgroupsro32[i].players);
			m->ReadOldResults(sgroupsro32[i], prev_matches, upcoming_matches, ro32.RoundOf(), false);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Copenhagen_Games_Spring_2015/Group_Stage_3&action=edit", true, false);
		vector<ScrapedGroup> sgroupsro16;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro16);
		for (uint i = 0; i < 4; i++) {
			auto m = ro16.GetMatch(sgroupsro16[i].players);
			m->ReadOldResults(sgroupsro16[i], prev_matches, upcoming_matches, ro16.RoundOf());
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Copenhagen_Games_Spring_2015/Playoffs&action=edit", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4 + 2; i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1; i++) {
			auto m = finals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*RandomAdvancement<32, 32> sortplayers;
		for (auto p : confs) {
		sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		for (uint i = (uint)confs.size(); i<32; i++) {
		Player *p = NULL;
		if (i<20) {
		p = good_players[rng.iRand64HQ() % good_players.size()];
		}
		else {
		while (p == NULL || p->player_id == 0) {
		p = &players[rng.iRand64HQ() % players.size()];
		}
		}
		bool found = false;
		for (auto a : sortplayers.advancing_players) {
		if (a == p) {
		found = true;
		break;
		}
		}
		if (found) {
		i--;
		continue;
		}
		sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/
		RandomAdvancement<16, 16> ro32adv;
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		//ro32.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);
		//ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class Gfinity1 : public TournamentBase
{
public:
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 8> invs;

	Gfinity1() : TournamentBase("Gfinity Spring 1", 50)
	{
		t_id = 50;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//finals.match_placing_to_tournament_placing[1] = 0;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[3] = ro16.points_for_placing[2] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;
		semifinals.best_of = 5;
		finals.best_of = 7;

		array<const char*, 8> sinvs = {
			"yoe;parting;p;kr", ";mc;p;kr", "myi;rain;p;kr", "jin air;terminator;p;kr",
			"liquid;bunny;t;dk",
			"eg;jaedong;z;kr", "tcm;soulkey;z;kr", "liquid;snute;z;no"
		};
		for (uint i = 0; i < sinvs.size(); i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}

		array<const char*, 16> sro16 = {
			"liquid;snute;z;no", ";targa;z;no", "psistm;true;z;kr", "yoe;parting;p;kr",
			"jin air;rogue;z;kr", ";mc;p;kr", "tcm;soulkey;z;kr", "myi;sacsri;z;kr",
			"liquid;bunny;t;dk", "root;hydra;z;kr", "myi;rain;p;kr", "dpix;fantasy;t;kr",
			"axiom;heart;t;kr", "eg;jaedong;z;kr", "jin air;terminator;p;kr", "sbenu;jjakji;t;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 8> sro8 = {
			"yoe;parting;p;kr", "tcm;soulkey;z;kr", "myi;rain;p;kr", "sbenu;jjakji;t;kr",
			"jin air;rogue;z;kr", "psistm;true;z;kr", "axiom;heart;t;kr", "root;hydra;z;kr"
		};
		for (uint i = 0; i < 8; i++) {
			Player *p = &SearchPlayer(sro8[i] DEBUGARGS);
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Gfinity_Spring_Masters_I&action=edit&section=6", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (uint i = 0; i < 4; i++) {
			auto m = ro16.GetMatch(sgroups[i].players);
			if (m) m->ReadOldResults(sgroups[i], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Gfinity_Spring_Masters_I&action=edit&section=11", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4; i++) {
			//smatches[i].timestamp = Timestamp("March 29, 2015 07:00 {{Abbr /CDT");
			auto m = quarterfinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4 + 2; i++) {
			//smatches[i].timestamp = Timestamp("March 29, 2015 07:00 {{Abbr /CDT");
			auto m = semifinals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4 + 2; i < 4 + 2 + 1; i++) {
			//smatches[i].timestamp = Timestamp("March 29, 2015 07:00 {{Abbr /CDT");
			auto m = finals.GetMatch(smatches[i].players);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -75);
			}
		}

		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class GfinityBase : public PlaceholderBase
{
public:
	GfinityBase(const char *tname, uint TID) : PlaceholderBase(tname, TID)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		PlaceholderBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.points_for_placing[3] = ro32.points_for_placing[2] = 0;
		ro16.points_for_placing[3] = ro16.points_for_placing[2] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;
		semifinals.best_of = 5;
		finals.best_of = 7;
		finals.match_placing_to_tournament_placing[0] = 1;
	}
};

class Gfinity2 : public GfinityBase
{
public:
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16groups;
	//array<Player*, 8> quals;
	array<Player*, 8> invs;

	Gfinity2() : GfinityBase("Gfinity Spring 2", 52) {}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		GfinityBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro16groups.points_for_placing[3] = ro16groups.points_for_placing[2] = 75;
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();
		finals.ResetPlacings();
		finals.match_placing_to_tournament_placing[0] = 1;
		array<const char*, 16> sro16 = {
			"eg;jaedong;z;kr", ";showtime;p;de", "roccat;hyun;z;kr", "psistm;true;z;kr",
			"liquid;bunny;t;dk", ";mc;p;kr", "axiom;heart;t;kr", "dpix;patience;p;kr",
			";bling;p;uk", "jin air;sos;p;kr", "myi;sacsri;z;kr", "at;uthermal;t;nl",
			"samsung;solar;z;kr", "root;hydra;z;kr", "yoe;san;p;kr", "jin air;symbol;z;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16groups.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 8> sinvs = {
			";mc;p;kr", "eg;jaedong;z;kr", "root;hydra;z;kr", "samsung;solar;z;kr", "jin air;sos;p;kr", "liquid;bunny;t;dk", ";showtime;p;de", ";bling;p;uk"
		};
		for (uint i = 0; i < sinvs.size(); i++) {
			auto p = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}

		array<const char*, 8> sro8 = {
			"eg;jaedong;z;kr", "dpix;patience;p;kr", "jin air;sos;p;kr", "root;hydra;z;kr",
			"liquid;bunny;t;dk", "psistm;true;z;kr", "samsung;solar;z;kr", "myi;sacsri;z;kr"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Gfinity_Spring_Masters_II&action=edit&section=6", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (uint i = 0; i < 4; i++) {
			if (sgroups[i].timestamp == 0) sgroups[i].timestamp = Timestamp("May 1, 2015 13:00 {{Abbr /CDT");
			auto m = ro16groups.GetMatch(sgroups[i].players);
			m->ReadOldResults(sgroups[i], prev_matches, upcoming_matches, ro16groups.RoundOf());
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Gfinity_Spring_Masters_II&action=edit&section=11", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		//smatches[0].players[1] = "patience";
		for (uint i = 0; i < 4; i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//array<bool, MAX_ALIGULAC_ID> player_map = { 0 };
		//player_map[0] = 1;
		/*RandomAdvancement<16, 0> sortro16;
		for (auto p : quals) {
		//player_map[p->aligulac_id] = 1;
		sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		}
		for (auto p : invs) {
		//player_map[p->aligulac_id] = 1;
		sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		}*/

		/*RandomAdvancement<32, 0> sortplayers;
		for (uint i = 0; i<32; i++) {
		Player *p = NULL;
		if (i<28) {
		p = good_players[rng.iRand64HQ() % good_players.size()];
		}
		else {
		p = &players[rng.iRand64HQ() % players.size()];
		}
		if (player_map[p->aligulac_id] == 1) {
		i--;
		continue;
		}
		player_map[p->aligulac_id] = 1;
		sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		RandomAdvancement<16, 16> ro32adv;*/
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		/*ro32.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);

		for (uint i = (uint)quals.size() + (uint)invs.size(); i < 16; i++) {
		sortro16.AddAdvancingPlayer(ro32adv.advancing_players[i], 0, 0, 0, 0, 0, rng);
		}*/

		//ro16groups.AcceptAdvancements(sortro16.advancing_players DEBUGARGS);
		ro16groups.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -75);
			}
		}

		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};
class Gfinity3 : public GfinityBase
{
public:
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16groups;
	array<Player*, 8> quals;
	array<Player*, 1> invs;

	Gfinity3() : GfinityBase("Gfinity Summer 1", 53) {}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		GfinityBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro16groups.points_for_placing[2] = ro16groups.points_for_placing[3] = 75;
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();
		finals.ResetPlacings();
		finals.match_placing_to_tournament_placing[0] = 1;

		array<const char*, 1> sinvs = {
			"yoe;parting;p;kr"
		};
		for (uint i = 0; i < sinvs.size(); i++) {
			auto p = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}
		array<const char*, 8> squals = {
			";first;p;kr", "mvp;departure;z;kr", "liquid;snute;z;no", "psistm;true;z;kr", "prime;keen;t;kr", "roccat;hyun;z;kr",
			"dpix;patience;p;kr", ";myungsik;p;kr"
		};
		for (uint i = 0; i < squals.size(); i++) {
			auto p = &SearchPlayer(squals[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quals[i] = p;
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		RandomAdvancement<16, 0> sortro16;
		for (auto p : quals) {
			player_map[p->player_id] = 1;
			sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		}
		for (auto p : invs) {
			player_map[p->player_id] = 1;
			sortro16.AddAdvancingPlayer(p, 0, 0, 0, 0, 0, rng);
		}

		RandomAdvancement<32, 0> sortplayers;
		for (uint i = 0; i < 32; i++) {
			Player *p = NULL;
			if (i < 28) {
				p = good_players[rng.iRand64HQ() % good_players.size()];
			}
			else {
				p = &players[rng.iRand64HQ() % players.size()];
			}
			if (player_map[p->player_id] == 1) {
				i--;
				continue;
			}
			player_map[p->player_id] = 1;
			sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		RandomAdvancement<16, 16> ro32adv;
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro32.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);

		for (uint i = (uint)quals.size() + (uint)invs.size(); i < 16; i++) {
			sortro16.AddAdvancingPlayer(ro32adv.advancing_players[i], 0, 0, 0, 0, 0, rng);
		}

		ro16groups.AcceptAdvancements(sortro16.advancing_players DEBUGARGS);
		ro16groups.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -75);
			}
		}

		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};
class Gfinity4 : public GfinityBase
{
public:
	Gfinity4() : GfinityBase("Gfinity Summer 2", 54) {}
};

class HKET3 : public TournamentBase
{
public:
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 4> invs;

	HKET3() : TournamentBase("3rd HKET", 55)
	{
		t_id = 55;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;
		semifinals.best_of = 5;
		finals.best_of = 7;

		array<const char*, 4> sinvs = {
			";bistork;p;hk", "hka;sen;z;tw", "yoe;ian;z;tw", "wayi;rex;z;tw"
		};
		for (uint i = 0; i < sinvs.size(); i++) {
			Player *p = &SearchPlayer(sinvs[i] DEBUGARGS);
			invs[i] = p;
		}

		array<const char*, 16> sro16 = {
			"wayi;ruin;p;kr", ";wil;t;tw", ";softball;t;tw", "hka;sen;z;tw",
			"sbenu;leenock;z;kr", ";miracle;p;tw", "wayi;fist;z;tw", "yoe;ian;z;tw",
			";musou;z;tw", ";vettel;p;tw", "yoe;san;p;kr", "wayi;rex;z;tw",
			"wayi;cheetos;z;tw", "yoe;parting;p;kr", "wayi;ak;z;tw", ";bistork;p;hk"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			Player *p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Hong_Kong_Esports_Tournament/3&action=edit&section=7", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 8; i++) {
			smatches[i].timestamp = Timestamp("May 31, 2015 00:00 {{Abbr /CDT");
			auto m = ro16.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 8; i < 8+4 && i<smatches.size(); i++) {
			smatches[i].timestamp = Timestamp("May 31, 2015 01:00 {{Abbr /CDT");
			auto m = quarterfinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8 + 4; i < 8 + 4 + 2 && i<smatches.size(); i++) {
			smatches[i].timestamp = Timestamp("June 13, 2015 00:00 {{Abbr /CDT");
			auto m = semifinals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 8 + 4 + 2; i < 8 + 4 + 2 + 1 && i<smatches.size(); i++) {
			smatches[i].timestamp = Timestamp("June 13, 2015 02:00 {{Abbr /CDT");
			auto m = finals.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);

		for (auto p1 : ro16adv.falling_players) {
			bool found = false;
			for (auto p2 : invs) {
				if (p1 == p2) {
					found = true;
					break;
				}
			}
			if (found) {
				sim.AddPlayerPoints(*p1, -75);
			}
		}

		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class DHTours : public DreamhackBase
{
public:
	//Round<96, SwissGroup, 16, RandomAdvancement<32, 32> > ro96;
	Round<64, SwissGroup, 16, RandomAdvancement<32, 32> > ro64;
	Round<32, SingleMatch, 16, StraightAdvancement<16, 16> > ro32b;
	//array<Player*, 96> confs;

	DHTours() : DreamhackBase("DreamHack Tours", 46)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		DreamhackBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro64.ClearPlacings();
		ro32.ClearPlacings();
		ro16.ResetPlacings();
		quarterfinals.ResetPlacings();
		semifinals.ResetPlacings();
		finals.ResetPlacings();
		finals.match_placing_to_tournament_placing[0] = 1;
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		//finals.ClearPlacings();
		/*array<const char*, 64> sro96 = {
			"invasion;miniraser;z;se", ";soyhi;z;fi", ";nepou;p;fr", ";invictus;z;fr",
			"myi;pengwin;z;ch", ";silent;p;fr", "tes;mark;z;it", ";crunchy;z;fr",
			"property;starnan;p;se", "tes;venividivins;z;it", "invasion;grumpy;z;fr", "myi;narcotic;t;it",
			";jeysen;p;fr", ";kirwa;z;fr", ";spearline;t;fr", ";zmk;z;fr",
			";revolver;z;ru", ";kafar;p;fr", ";konekosan;t;fr", ";totoro;t;fr",
			"property;mekar;p;se", ";rotab;z;fr", ";frostmind;z;fr", "nb;hinoipl;z;fr",
			"tes;stareagle;t;it", ";Symphoniaa;t;fr", "nb;dylayn;z;fr", "aaa;chubz;p;fr",
			"invasion;harstem;p;nl", ";muyfresh;t;fr", "rblood;littlepanda;z;fr", "rodg;funk;p;ch",
			"aaa;psionic;p;fr", "at;Gannicus;t;nl", ";davdou;z;fr", ";behemoth;p;fr",
			"acer;bly;z;ua", ";odelay;t;fr", ";kenjo;t;fr", ";mraptor;p;fr",
			"at;jona;z;nl", "rodg;jack;t;fr", ";breakya;z;ch", ";widowmaker;z;fr",
			"acer;scarlett;z;ca", ";vacsaint;z;fr", ";superhofmann;z;it", ";gogogadgeto;z;fr",
			"myi;zervas;z;ch", ";sbr;z;fr", "aaa;dns;p;fr", ";maczerguey;z;fr",
			"tsk;verdi;p;ru", ";oros;z;fr", ";srilo;z;fr", ";godu;z;fr",
			"millenium;dayshi;t;fr", ";deborah;z;es", ";white-ra;p;ua", ";sapty;t;fr",
			"myi;ptitdrogo;p;fr", ";poucky;z;fr", "invasion;quent;p;fr", "nb;Imre;t;fr",
		};
		for (uint i = 0; i < sro96.size(); i++) {
			auto p = &SearchPlayer(sro96[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro96.matches[i/4].players[i%4] = p;
		}*/

		array<const char*, 64> sro64 = {
			"root;hydra;z;kr", ";kirwa;z;fr", ";firecake;z;fr", "aaa;chubz;p;fr",
			"roccat;hyun;z;kr", ";kenjo;t;fr", "millenium;lilbow;p;fr", "invasion;miniraser;z;se",
			";mc;p;kr", ";hinoipl;p;fr", "millenium;marinelord;t;fr", "property;starnan;p;se",
			"myi;sacsri;z;kr", "tes;mark;z;it", "eg;huk;p;ca", "millenium;dayshi;t;fr",
			"mvp;gumiho;t;kr", ";oros;z;fr", "myi;serral;z;fi", "invasion;harstem;p;nl",
			"acer;mma;t;kr", ";revolver;z;ru", "liquid;mana;p;pl", "acer;scarlett;z;ca",
			"millenium;forgg;t;kr", "at;Gannicus;t;nl", "liquid;snute;z;no", "myi;ptitdrogo;p;fr",
			"dpix;patience;p;kr", "rodg;jack;t;fr", "wayi;ruin;p;kr", "aaa;dns;p;fr",
			"sbenu;leenock;z;kr", "nb;Imre;t;fr", ";targa;z;no", "mcon;jona;z;nl",
			"dpix;fantasy;t;kr", ";sbr;z;fr", "liquid;tlo;z;de", "aaa;psionic;p;fr",
			"mvp;marineking;t;kr", "tes;diord;t;it", "liquid;bunny;t;dk", "meltdown;stephano;z;fr",
			"axiom;heart;t;kr", "rodg;funk;p;ch", ";petraeus;z;nz", "isimba;verdi;p;ru",
			"yoe;parting;p;kr", ";white-ra;p;ua", "at;uthermal;t;nl", "myi;pengwin;z;ch",
			"myi;rain;p;kr", ";venividivins;z;it", ";welmu;p;fi", "property;mekar;p;se",
			"yoe;san;p;kr", ";soyhi;z;fi", "acer;bly;z;ua", "eg;xenocider;t;us",
			"psistm;true;z;kr", "nb;dylayn;z;fr", "property;zanster;z;se", ";jeysen;p;fr",
		};
		for (uint i = 0; i < sro64.size(); i++) {
			auto p = &SearchPlayer(sro64[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro64.matches[i/4].players[i%4] = p;
		}

		array<const char *, 32> sro32 = {
			"yoe;parting;p;kr", "property;zanster;z;se", "liquid;mana;p;pl", ";firecake;z;fr",
			"wayi;ruin;p;kr", "yoe;san;p;kr", "myi;rain;p;kr", "millenium;dayshi;t;fr",
			";mc;p;kr", "roccat;hyun;z;kr", "dpix;fantasy;t;kr", "liquid;bunny;t;dk",
			"isimba;verdi;p;ru", "liquid;snute;z;no", ";targa;z;no", "invasion;harstem;p;nl",
			"mvp;gumiho;t;kr", "sbenu;leenock;z;kr", "millenium;forgg;t;kr", "axiom;heart;t;kr",
			"mvp;marineking;t;kr", "liquid;tlo;z;de", "millenium;lilbow;p;fr", "millenium;marinelord;t;fr",
			"myi;sacsri;z;kr", ";welmu;p;fi", "acer;bly;z;ua", "dpix;patience;p;kr",
			"root;hydra;z;kr", "acer;scarlett;z;ca", "psistm;true;z;kr", "at;uthermal;t;nl"
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			ro32b.matches[i / 2].players[i % 2] = p;
		}

		/*string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Tours/Group_Stage_1&action=edit", false, false);
		vector<ScrapedGroup> ro96groups;
		ScrapeLiquipediaGroupsDH(res, t_id, ro96groups);
		for (uint i = 0; i < 16; i++) {
			auto m = ro96.GetMatch(ro96groups[i].players);
			assert(m != NULL);
			m->ReadOldResults(ro96groups[i], prev_matches, upcoming_matches, ro96.RoundOf());
		}*/
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Tours/Group_Stage_2&action=edit", true, false);
		vector<ScrapedGroup> ro64groups;
		ScrapeLiquipediaGroupsDH(res, t_id, ro64groups);
		ro64groups[2].players[3] = "hinoipl";
		for (uint i = 0; i < 16; i++) {
			auto m = ro64.GetMatch(ro64groups[i].players);
			assert(m != NULL);
			m->ReadOldResults(ro64groups[i], prev_matches, upcoming_matches, ro64.RoundOf());
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Tours/Playoffs&action=edit", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 16; i++) {
			auto m = ro32b.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro32b.RoundOf());
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 16; i < 16+8 && i<smatches.size(); i++) {
			auto m = ro16.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, ro16.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8; i < 16 + 8+4 && i<smatches.size(); i++) {
			auto m = quarterfinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8+4; i < 16 + 8 + 4+2 && i<smatches.size(); i++) {
			auto m = semifinals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8+4+2; i < 16 + 8 + 4+2+1 && i<smatches.size(); i++) {
			auto m = finals.GetMatch(smatches[i].players);
			assert(m != NULL);
			m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*array<bool, MAX_PLAYER_ID> player_map = { 0 };
		player_map[0] = 1;
		RandomAdvancement<64, 0> sortplayers;
		for (uint i = 0; i<64; i++) {
			Player *p = NULL;
			if (i < 500) {
				p = confs[rng.iRand64HQ() % confs.size()];
			}
			else if (i<60) {
				p = good_players[rng.iRand64HQ() % good_players.size()];
			}
			else {
				p = &players[rng.iRand64HQ() % players.size()];
			}
			if (player_map[p->player_id] == 1) {
				i--;
				continue;
			}
			player_map[p->player_id] = 1;
			sortplayers.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/
		//RandomAdvancement<32, 32> ro96adv;
		RandomAdvancement<32, 32> ro64adv;
		StraightAdvancement<16, 16> ro32adv;
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		//ro96.predict(sim, t_id, ro96adv, rng DEBUGARGS);
		/*for (uint i = 0; i < ro96adv.advancing_players.size(); i++) {
			ro64.matches[i / 2].players[(i % 2) * 2 + 1] = ro96adv.advancing_players[i];
		}*/
		//ro64.AcceptAdvancements(sortplayers.advancing_players DEBUGARGS);
		ro64.predict(sim, t_id, ro64adv, rng DEBUGARGS);
		//ro32b.AcceptAdvancements(ro64adv.advancing_players DEBUGARGS);
		ro32b.predict(sim, t_id, ro32adv, rng DEBUGARGS);
		ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class DHValencia : public TournamentBase
{
public:
	//Round<96, SwissGroup, 16, RandomAdvancement<32, 32> > ro96;
	//Round<64, SwissGroup, 16, RandomAdvancement<32, 32> > ro64;
	Round<32, SingleMatch, 16, StraightAdvancement<16, 16> > ro32;
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	DHValencia(const char *name, uint TID) : TournamentBase(name, TID)
	{
		t_id = TID;
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro96.ClearPlacings();
		//ro64.ClearPlacings();
		//ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		finals.match_placing_to_tournament_placing[1] = 2;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		finals.best_of = 5;

		/*array<const char*, 64> sro96 = {
			"estar;Botvinnik;T;Es", ";mUOz;T;Es", ";hofobru;T;Es", "Esos;Varrittor;P;Es",
			";BlackKnight;z;es", "Esos;PrinzZ;z;Es", ";Balrog;P;Fr", ";Rote;t;es",
			"Menace;ZhuGeLiang;Z;Fi", "aaa;MiNiMaTh;Z;Fr", "AT;FeaR;P;De", ";Velego;P;Es",
			";ExCeN;Z;Es", ";Mzrk;Z;Es", "Punchline!;Clem;t;Fr", ";Borrex;Z;Es",
			"Aaa;Chubz;P;Fr", ";Kellermann;Z;Es", ";dom;T;Es", ";Seraphim;P;Gr",
			";Thariel;p;es", ";thenanox;Z;Es", "invasion;lambo;z;de", ";Teinez;T;Es",
			";jeysen;P;Fr", ";MindTriiks;T;Es", ";SmoNKa;P;Es", ";Exalon;Z;Es",
			";Jandro;T;Es", ";Kenro;P;Es", ";WooP;z;es", ";Lutor;T;Es",
			"aaa;DnS;P;Fr", ";NeXuS;P;Es", ";Anthen;P;Es", ";Deborah;Z;Es",
			"aaa;PsiOniC;P;Fr", ";Lysander;Z;Es", ";Rotab;Z;Fr", ";Ivacks;P;Es",
			"dpix;Majestic;P;Es", ";Inok;Z;Es", ";Protosito;P;Es", "esos;monxz;z;es",
			";WhiteRa;P;Ua", ";Crispindrix;z;es", "tsk;Ziggy;T;Pl", "Esos;Hunter;P;Es",
			"Grav;WaKa;Z;Es", ";Rubo;T;Es", "Punchline!;Dext;T;Fr", ";Nugui;Z;Es",
			"PSISTM;arium;P;Us", ";Frikisama;R;Es", "Esos;kamip;T;Es", ";Insomnia;P;Es",
			"myi;Reynor;Z;It", "nb;hinoipl;z;fr", "esos;SouLeer;Z;Es", ";DaniA;R;Es",
			"myi;PengWin;Z;Ch", ";IBIMarine;p;Es", "Dpix;Elroye;Z;Ch", ";Juanan;z;es"
		};
		for (uint i = 0; i < sro96.size(); i++) {
			Player *p = &players[0];
			try { p = &SearchPlayer(sro96[i] DEBUGARGS); }
			catch (...){}
			if (p->player_id == 0) {
				vector<string> split;
				SplitBy(sro96[i], ';', split);
				const char* team = NULL;
				if (split[0].length()) team = split[0].c_str();
				char race = R;
				string srace = ToLower(split[2]);
				if (srace == "t") race = T;
				if (srace == "z") race = Z;
				if (srace == "p") race = P;
				p=&MakeNewb(team, split[1].c_str(), split[3], race);
			}
			if (p->league == 0) p->league = t_id;
			ro96.matches[i/4].players[i%4] = p;
		}*/

		/*array<const char*, 32> sro64 = {
			"EnVy;viOLet;Z;Kr", "Axiom;Ryung;T;Kr",
			"dpix;FanTaSy;T;Kr", "Property;MorroW;T;Se",
			"myi;Sacsri;Z;Kr", "tsk;Verdi;P;Ru",
			"Liquid;Bunny;T;Dk", "Millenium;Lilbow;P;Fr",
			"Jin Air;Symbol;Z;Kr", "myi;Serral;Z;Fi",
			"Jin Air;Pigbaby;P;Kr", "Liquid;MaNa;P;Pl",
			"Yoe;San;P;Kr", "myi;PtitDrogo;P;Fr",
			"Liquid;HerO;P;Kr", "Millenium;MarineLorD;T;Fr",
			"MVP;GuMiho;T;Kr", "Invasion;Harstem;P;Nl",
			"psistm;true;Z;Kr", "Millenium;Dayshi;T;Fr",
			"Jin Air;Terminator;P;Kr", "Liquid;TLO;Z;De",
			"SBENU;Curious;Z;Kr", "AT;Jona;Z;Nl",
			"Liquid;TaeJa;T;Kr", "Liquid;Ret;Z;Nl",
			"sbenu;leenock;Z;Kr", "eg;HuK;P;Ca",
			"ROCCAT;HyuN;Z;Kr", "myi;Blysk;P;Sg",
			"dpix;Patience;P;Kr", "property;zanster;Z;Se",
		};
		for (uint i = 0; i < sro64.size(); i++) {
			auto p = &SearchPlayer(sro64[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro64.matches[i / 2].players[(i % 2) * 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Valencia/Group_Stage_2&action=edit", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroupsDH(res, t_id, sgroups);
		for (auto &g : sgroups) {
			auto m = ro64.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro64.RoundOf());
			assert(m != NULL);
		}*/

		array<const char*, 32> sro32 = {
			"Jin Air;Terminator;P;Kr", "MVP;GuMiho;T;Kr", "dpix;Patience;P;Kr", "myi;Blysk;P;Sg",
			"dpix;FanTaSy;T;Kr", "Liquid;MaNa;P;Pl", "myi;PtitDrogo;P;Fr", "Liquid;Ret;Z;Nl",
			"isimba;Verdi;P;Ru", ";Mzrk;Z;Es", "Millenium;MarineLorD;T;Fr", "invasion;lambo;z;de",
			"myi;Serral;Z;Fi", "Liquid;Bunny;T;Dk", "SBENU;Curious;Z;Kr", "Millenium;Dayshi;T;Fr",
			"psistm;true;Z;Kr", "mcon;Jona;Z;Nl", "PSISTM;arium;P;Us", "Jin Air;Symbol;Z;Kr",
			"sbenu;leenock;Z;Kr", "Liquid;HerO;P;Kr", "Axiom;Ryung;T;Kr", "myi;Sacsri;Z;Kr",
			"Liquid;TaeJa;T;Kr", "Yoe;San;P;Kr", "Jin Air;Pigbaby;P;Kr", "Property;MorroW;T;Se",
			"ROCCAT;HyuN;Z;Kr", "property;zanster;Z;Se", "Invasion;Harstem;P;Nl", "Liquid;TLO;Z;De"
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Valencia/Playoffs&action=edit&section=1", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 16;i++) {
			auto &g = smatches[i];
			auto m = ro32.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 16; i < 16+8 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8; i < 16 + 8+4 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8+4; i < 16 + 8+4+2 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8+4+2; i < 16 + 8+4+2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		/*RandomAdvancement<32, 32> ro96adv;
		RandomAdvancement<32, 32> ro64adv;*/
		StraightAdvancement<16, 16> ro32adv;
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		/*ro96.predict(sim, t_id, ro96adv, rng DEBUGARGS);

		//ro64.AcceptAdvancements(ro128adv.advancing_players DEBUGARGS);
		for (uint i = 0; i < ro96adv.advancing_players.size(); i++) {
			ro64.matches[i / 2].players[(i % 2) * 2 + 1] = ro96adv.advancing_players[i];
		}
		ro64.predict(sim, t_id, ro64adv, rng DEBUGARGS);

		ro32.AcceptAdvancements(ro64adv.advancing_players DEBUGARGS);*/
		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);
		ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class DHStockholm : public TournamentBase
{
public:
	//Round<96, SwissGroup, 16, RandomAdvancement<32, 32> > ro96;
	//Round<64, SwissGroup, 16, RandomAdvancement<32, 32> > ro64;
	Round<32, SingleMatch, 16, StraightAdvancement<16, 16> > ro32;
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	DHStockholm() : TournamentBase("DreamHack Stockholm", 48)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro96.ClearPlacings();
		//ro64.ClearPlacings();
		finals.match_placing_to_tournament_placing[1] = 2;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		finals.best_of = 5;

		/*array<const char*, 64> sro96 = {
			"Property;Mekar;P;se", ";walkover", ";HeltEn;T;no", ";breakya;z;ch",
			"nb;hinO;Z;fr", ";walkover", ";BomBur;Z;se", "Rblood;PappiJoe;P;dk",
			"ecv;Snovski;P;dk", ";Gimme;P;no", ";Ramses;Z;se", ";Reaver;P;no",
			"Darkstar;Multi;P;se", "karnage;InZaNe;T;de", ";walkover", ";Rodzyn;P;pl",
			"mcon;Jona;Z;nl", ";dead;R;us", "dpix;Elroye;Z;ch", "MiA;CheeseKing;P;hr",
			"mYi;Zervas;Z;ch", ";SpaceDucK;P;se", "mYi;Reynor;Z;it", ";Chendar;Z;se",
			"mYi;Winter;Z;se", ";Wanji;T;se", "rodg;FunK;P;ch", "skill forge;Tocon;T;pl",
			"ROOT;SolO;Z;no", ";whitefenix;z;se", ";Gerald;P;pl", ";Celadon;T;no",
			"Karnage;Namshar;Z;se", "Karnage;PuPu;Z;fi", ";frozz;T;hr", "ECV;Cladorhiza;Z;uk",
			"tes;Guru;Z;pl", ";walkover", "nb;Basior;T;pl", ";Civi;Z;no",
			";SortOf;Z;se", ";NomaK;Z;uk", ";Hantypen;t;no", "isIMBA;Miszu;T;pl",
			";SpaceMarine;P;dk", ";WAWWSamet;P;se", ";walkover", "tsk;Ziggy;T;pl",
			"ROOT;Revolution;Z;uk", "ECV;Pox;p;dk", ";Aggression;T;dk", ";MrWhisper;Z;uk",
			"Property;StarNaN;P;se", ";Scythe;T;no", ";Triaz;Z;no", ";NightOwl;Z;se",
			"Karnage;Evire;P;no", ";walkover", "ECV;Fazz;Z;nl", ";plzleaveduck;Z;us",
			"Menace;ZhuGeLiang;Z;fi", ";Damrud;Z;pl", "mYi;PengWin;Z;ch", "sgeht;Delking;P;de"
		};
		for (uint i = 0; i < sro96.size(); i++) {
			auto p = &SearchPlayer(sro96[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro96.matches[i/4].players[i%4] = p;
		}*/

		/*array<const char*, 64> sro64 = {
			 "Tes;Elazer;Z;pl",
			 "Liquid;MaNa;P;pl",
			 ";iaguz;T;au",
		};
		for (uint i = 0; i < sro64.size(); i++) {
			auto p = &SearchPlayer(sro64[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro64.matches[i / 4].players[i % 4] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Stockholm/Group_Stage_2&action=edit", false, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroupsDH(res, t_id, sgroups);
		for (auto &g : sgroups) {
			auto m = ro64.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro64.RoundOf());
			assert(m != NULL);
		}*/

		array<const char*, 32> sro32 = {
			"ROCCAT;HyuN;Z;kr", "eg;HuK;P;ca", "dpix;FanTaSy;T;kr", ";Welmu;P;fi", "psistm;true;Z;kr", "mYi;PtitDrogo;P;fr", "mYi;Rain;P;kr", "Liquid;HerO;P;kr",
			"sbenu;leenock;Z;kr", "Menace;ZhuGeLiang;Z;fi", "SKT T1;Dark;Z;kr", ";Apocalypse;T;kr", "Samsung;Reality;T;kr", "Property;MorroW;T;se", "eg;Jaedong;Z;kr", ";Gamja;Z;kr",
			"Invasion;Harstem;P;nl", "Property;Zanster;Z;se", "Jin air;sOs;P;kr", "Invasion;Lambo;Z;de", "mYi;Sacsri;Z;kr", "mYi;Serral;Z;fi", "sbenu;Curious;Z;kr", ";DeMusliM;T;uk",
			"Samsung;Hurricane;P;kr", "nb;hinO;Z;fr", "Samsung;Solar;Z;kr", "invasion;miniraser;Z;se", "Liquid;Snute;Z;no", ";elfi;P;fi", "Samsung;Armani;Z;kr", "isIMBA;Miszu;T;pl",
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_DreamHack_Open/Stockholm/Playoffs&action=edit&section=1", false, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 16; i++) {
			auto &g = smatches[i];
			auto m = ro32.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 16; i < 16+8; i++) {
			auto &g = smatches[i];
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8; i < 16 + 8+4 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8+4; i < 16 + 8 + 4+2 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8 + 4+2; i < 16 + 8 + 4 + 2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		//RandomAdvancement<32, 32> ro96adv;
		//RandomAdvancement<32, 32> ro64adv;
		StraightAdvancement<16, 16> ro32adv;
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		/*ro96.predict(sim, t_id, ro96adv, rng DEBUGARGS);
		for (uint i = 0; i < 32; i++) {
			ro64.matches[i / 2].players[(i % 2)*2 + 1] = ro96adv.advancing_players[i];
		}*/
		//ro64.predict(sim, t_id, ro64adv, rng DEBUGARGS);
		//ro32.AcceptAdvancements(ro64adv.advancing_players DEBUGARGS);
		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);
		ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class ASUSSummer : public DreamhackBase
{
public:
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16groups;

	ASUSSummer() : DreamhackBase("ASUS ROG Summer", 59)
	{
		//t_id = 59;
		//TournamentNames[t_id] = "ASUS ROG Summer";
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		DreamhackBase::init(prev_matches, upcoming_matches DEBUGARGS);
		ro32.points_for_placing[0] = ro32.points_for_placing[1] = ro32.points_for_placing[2] = ro32.points_for_placing[3] = 0;
		ro16groups.points_for_placing[2] = ro16groups.points_for_placing[3] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;

		array<const char*, 32> sro32 = {
			"roccat;hyun;z;kr", ";denver;z;fr", ";apocalypse;t;kr", ";elfi;p;fi",
			"psistm;true;z;kr", "karnage;calt;t;fi", "millenium;marinelord;t;fr", "property;zanster;z;se",
			"liquid;bunny;t;dk", "property;starnan;p;se", ";welmu;p;fi", ";gamja;z;kr",
			"millenium;lilbow;p;fr", "menace;zhugeliang;z;fi", "liquid;ret;z;nl", "property;naniwa;p;se",
			"dpix;fantasy;t;kr", "karnage;namshar;z;se", "prime;keen;t;kr", "myi;serral;z;fi",
			"dpix;patience;p;kr", "tes;guru;z;pl", "prime;bboongbboong;z;kr", "at;uthermal;t;nl",
			"liquid;snute;z;no", ";fuzer;t;fi", ";stardust;p;kr", "myi;ptitdrogo;p;fr",
			"mvp;losira;z;kr", "aaa;minimath;z;fr", "liquid;tlo;z;de", ";happy;t;ru"
		};
		for (uint i = 0; i < sro32.size(); i++) {
			auto p = &SearchPlayer(sro32[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro32.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 16> sro16 = {
			"roccat;hyun;z;kr", ";welmu;p;fi", "prime;bboongbboong;z;kr", "liquid;snute;z;no",
			"psistm;true;z;kr", "liquid;tlo;z;de", "millenium;lilbow;p;fr", "dpix;fantasy;t;kr",
			"prime;keen;t;kr", "myi;ptitdrogo;p;fr", "at;uthermal;t;nl", "property;naniwa;p;se",
			"liquid;bunny;t;dk", "millenium;marinelord;t;fr", "mvp;losira;z;kr", ";apocalypse;t;kr"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16groups.matches[i / 4].players[i % 4] = p;
		}
		array<const char*, 8> sro8 = {
			"roccat;hyun;z;kr", "psistm;true;z;kr", "myi;ptitdrogo;p;fr", ";apocalypse;t;kr",
			"dpix;fantasy;t;kr", "prime;bboongbboong;z;kr", "mvp;losira;z;kr", "prime;keen;t;kr"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=ASUS_ROG_Summer_2015/Group_Stage/1&action=edit&section=T-1", true, false);
		vector<ScrapedGroup> sgroupsro32;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro32);
		for (auto &g : sgroupsro32) {
			auto m = ro32.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro32.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=ASUS_ROG_Summer_2015/Group_Stage/2&action=edit&section=T-1", true, false);
		vector<ScrapedGroup> sgroupsro16;
		ScrapeLiquipediaGroups(res, t_id, sgroupsro16);
		for (auto &g : sgroupsro16) {
			auto m = ro16groups.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16groups.RoundOf());
			assert(m != NULL);
		}
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=ASUS_ROG_Summer_2015/Playoffs&action=edit&section=T-1", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 0; i < 4;i++) {
			auto &g = smatches[i];
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 4; i < 4+2; i++) {
			auto &g = smatches[i];
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 4+2; i < 4 + 2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 16> ro32adv;
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro32.predict(sim, t_id, ro32adv, rng DEBUGARGS);
		//ro16.AcceptAdvancements(ro32adv.advancing_players DEBUGARGS);
		ro16groups.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class MSI : public TournamentBase
{
public:
	//Round<32, SwissGroup, 8, RandomAdvancement<16, 16> > ro32;
	Round<16, SwissGroup, 4, RandomAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	array<Player*, 7> invs;
	//array<Player*, 10> quals;

	MSI() : TournamentBase("MSI Summer Masters", 61)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		//ro32.ClearPlacings();
		//ro16.ClearPlacings();
		//quarterfinals.ClearPlacings();
		//semifinals.ClearPlacings();
		finals.match_placing_to_tournament_placing[1] = 2;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[3] = ro16.points_for_placing[2] = 125;
		quarterfinals.points_for_placing[1] = 250;
		semifinals.points_for_placing[1] = 375;
		finals.points_for_placing[1] = 500;
		finals.points_for_placing[0] = 750;
		finals.best_of = 5;

		array<const char*, 7> sinvs = { 
			"yoe;parting;p;kr", "samsung;solar;z;kr", "roccat;hyun;z;kr", "eg;jaedong;z;kr", "envy;violet;z;kr", "yoe;ian;z;tw", "jin air;sos;p;kr" };
		for (uint i = 0; i < sinvs.size(); i++) {
			auto p = &SearchPlayer(sinvs[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			invs[i] = p;
		}

		array<const char*, 16> sro16 = {
			"root;hydra;z;kr", "ig;macsed;p;cn", "skt t1;innovation;t;kr", "yoe;has;p;tw",
			"yoe;parting;p;kr", "yoe;ian;z;tw", "roccat;hyun;z;kr", ";firecake;z;fr",
			"jin air;sos;p;kr", "zoo;toodming;z;cn", "samsung;solar;z;kr", "invasion;harstem;p;nl",
			"cm storm;polt;t;kr", "envy;violet;z;kr", "eg;jaedong;z;kr", ";petraeus;z;nz"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i/4].players[i%4] = p;
		}
		
		array<const char*, 8> sro8 = {
			"samsung;solar;z;kr", "eg;jaedong;z;kr", "root;hydra;z;kr", "yoe;parting;p;kr",
			"jin air;sos;p;kr", "envy;violet;z;kr", "skt t1;innovation;t;kr", "roccat;hyun;z;kr"
		};
		for (uint i = 0; i < sro8.size(); i++) {
			auto p = &SearchPlayer(sro8[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			quarterfinals.matches[i/2].players[i%2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_MSI_Masters_Gaming_Arena&action=edit&section=8", true, false);
		vector<ScrapedGroup> sgroups;
		ScrapeLiquipediaGroups(res, t_id, sgroups);
		for (auto &g : sgroups) {
			if (g.timestamp == 0) g.timestamp = Timestamp("August 30, 2015 10:00 {{Abbr /CDT");
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		
		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_MSI_Masters_Gaming_Arena&action=edit&section=13", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for(uint i=0;i<4;i++) {
			auto &g = smatches[i];
			if (g.timestamp == 0) g.timestamp = Timestamp("August 31, 2015 12:00 {{Abbr /CDT");
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for(uint i=4;i<4+2 && i<smatches.size();i++) {
			auto &g = smatches[i];
			if (g.timestamp == 0) g.timestamp = Timestamp("August 31, 2015 13:00 {{Abbr /CDT");
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
			assert(m != NULL);
		}
		predict(sim, rng DEBUGARGS);
		for(uint i=4+2;i<4+2+1 && i<smatches.size();i++) {
			auto &g = smatches[i];
			if (g.timestamp == 0) g.timestamp = Timestamp("August 31, 2015 15:00 {{Abbr /CDT");
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
			assert(m != NULL);
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<16, 0> ro16sort;
		RandomAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		for (auto p1 : ro16adv.falling_players) {
			for (auto p2 : invs) {
				if (p1 == p2) {
					sim.AddPlayerPoints(*p2, -125);
					break;
				}
			}
		}
		//quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};

class ForGamers : public TournamentBase
{
public:
	Round<16, SingleMatch, 8, StraightAdvancement<8, 8> > ro16;
	Round<8, SingleMatch, 4, StraightAdvancement<4, 4> > quarterfinals;
	Round<4, SingleMatch, 2, StraightAdvancement<2, 2> > semifinals;
	Round<2, SingleMatch, 1, StraightAdvancement<1, 1> > finals;

	ForGamers() : TournamentBase("4Gamers", 64)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches DEBUGARGSDEF)
	{
		ro16.best_of = 5;
		quarterfinals.best_of = 5;
		semifinals.best_of = 5;
		finals.best_of = 7;
		finals.match_placing_to_tournament_placing[0] = 1;
		ro16.points_for_placing[1] = 75;
		quarterfinals.points_for_placing[1] = 100;
		semifinals.points_for_placing[1] = 150;
		finals.points_for_placing[1] = 200;
		finals.points_for_placing[0] = 300;

		array<const char*, 16> sro16 = {
			"roccat;hyun;z;kr", "yoe;nice;p;tw", ";pet;z;kr", "yoe;cell;z;tw",
			"yoe;parting;p;kr", ";nado;t;kr", "yoe;has;p;tw", "wayi;fist;z;tw",
			"sbenu;bomber;t;kr", "wayi;ak;z;tw", "ig;jim;p;cn", "wayi;rex;z;tw",
			"ig;rushcrazy;p;cn", "wayi;cheetos;z;tw", "ig;macsed;p;cn", "yoe;ian;z;tw"
		};
		for (uint i = 0; i < sro16.size(); i++) {
			auto p = &SearchPlayer(sro16[i] DEBUGARGS);
			if (p->league == 0) p->league = t_id;
			ro16.matches[i / 2].players[i % 2] = p;
		}
		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=4Gamers_StarCraft_II_Tournament&action=edit&section=5", true, false);
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, t_id, smatches);
		for (uint i = 16; i < 16 + 8; i++) {
			auto &g = smatches[i];
			if (i < 16 + 4 && g.timestamp==0) g.timestamp = Timestamp("August 7, 2015 3:00 {{Abbr /PDT");
			else if(g.timestamp==0) g.timestamp = Timestamp("August 8, 2015 3:00 {{Abbr /PDT");
			auto m = ro16.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, ro16.RoundOf());
			assert(m != NULL);
		}
		Simulation sim;
		RayRand64 rng;
		predict(sim, rng DEBUGARGS);
		for (uint i = 16+8; i < 16 + 8+4 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			if(g.timestamp==0) g.timestamp = Timestamp("August 9, 2015 3:00 {{Abbr /PDT");
			auto m = quarterfinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, quarterfinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8+4; i < 16 + 8 + 4+2 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			if(g.timestamp==0) g.timestamp = Timestamp("September 5, 2015 0:00 {{Abbr /CST");
			auto m = semifinals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, semifinals.RoundOf());
		}
		predict(sim, rng DEBUGARGS);
		for (uint i = 16 + 8 + 4+2; i < 16 + 8 + 4 + 2+1 && i<smatches.size(); i++) {
			auto &g = smatches[i];
			if(g.timestamp==0) g.timestamp = Timestamp("September 5, 2015 2:00 {{Abbr /CST");
			auto m = finals.GetMatch(g.players);
			if (m) m->ReadOldResults(g, prev_matches, upcoming_matches, finals.RoundOf());
		}
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		StraightAdvancement<8, 8> ro16adv;
		StraightAdvancement<4, 4> quartersadv;
		StraightAdvancement<2, 2> semisadv;
		StraightAdvancement<1, 1> finalsadv;

		ro16.predict(sim, t_id, ro16adv, rng DEBUGARGS);
		quarterfinals.AcceptAdvancements(ro16adv.advancing_players DEBUGARGS);
		quarterfinals.predict(sim, t_id, quartersadv, rng DEBUGARGS);
		semifinals.AcceptAdvancements(quartersadv.advancing_players DEBUGARGS);
		semifinals.predict(sim, t_id, semisadv, rng DEBUGARGS);
		finals.AcceptAdvancements(semisadv.advancing_players DEBUGARGS);
		finals.predict(sim, t_id, finalsadv, rng DEBUGARGS);
	}
};


class WCSS1 : public WCSBase
{
public:
	array<Player*, 28> euquals;
	array<Player*, 16> amquals;
	//array<Player*, 8> amquals2;
	array<Player*, 8> cnquals;
	array<Player*, 4> seaquals;
	array<Player*, 4> laquals;
	array<Player*, 4> twquals;

	SPOTVS1 spotv;

	WCSS1() : WCSBase(1), spotv("SSL S1", 7)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		WCSBase::init(prev_matches, upcoming_matches, frees DEBUGARGS);
		spotv.init_s1(prev_matches, upcoming_matches DEBUGARGS);
		codes.init_s1(prev_matches, upcoming_matches, frees DEBUGARGS);
		quals.init_s1(DEBUGARG);
		//prem.init_s1(prev_matches, upcoming_matches DEBUGARGS);//do this after the inline challenger init

		/*array<const char*, 8> samquals = { "root;hydra;z;kr", "col;qxc;t;us", "f3;bails;p;us", ";violet;z;kr",
		";kane;z;ca", "f3;puck;p;us", "root;masa;t;ca", "root;astrea;p;us"
		};
		for (uint i = 0; i < samquals.size(); i++) {
		Player *p = &SearchPlayer(samquals[i] DEBUGARGS);
		p->league = chal.t_id;
		amquals[i] = p;
		frees.remove_vec(frees.northamericans, p);
		}
		array<const char*, 8> samquals2 = { "cm storm;polt;t;kr", "eg;xenocider;t;us", ";guitarcheese;z;us", ";state;p;us",
		"acer;scarlett;z;ca", "grav;semper;t;ca", "eg;suppy;z;us", "eg;huk;p;ca" };
		for (uint i = 0; i < samquals2.size(); i++) {
		Player *p = &SearchPlayer(samquals2[i] DEBUGARGS);
		p->league = chal.t_id;
		amquals2[i] = p;
		frees.remove_vec(frees.northamericans, p);
		}*/
		array<const char*, 28> seuquals = { ";happy;t;ru", "dpix;majestic;p;es", ";welmu;p;fi", "tes;beastyqt;t;rs",
			"invasion;harstem;p;nl", "myi;serral;z;fi", "liquid;mana;p;pl", "mouz;heromarine;t;de",
			"at;uthermal;t;nl", "acer;bly;z;ua", ";showtime;p;de", "millenium;lilbow;p;fr", ";bling;p;uk",
			"property;morrow;t;se", "liquid;tlo;z;de", "liquid;ret;z;nl",
			";targa;z;no", "cascade;kas;t;ua", "liquid;snute;z;no", "aaa;psionic;p;fr", ";starbuck;z;si", ";firecake;z;fr",
			"millenium;forgg;t;kr", "millenium;marinelord;t;fr", "acer;nerchio;z;pl", "property;naniwa;p;se", "liquid;bunny;t;dk", "myi;ptitdrogo;p;fr" };
		for (uint i = 0; i < seuquals.size(); i++) {
			Player *p = &SearchPlayer(seuquals[i] DEBUGARGS);
			p->league = chal.t_id;
			euquals[i] = p;
			frees.remove_vec(frees.europeans, p);
			chal.ro64.matches[(i / 2)].players[i % 2] = p;
		}
		array<const char*, 16> samquals = { "eg;huk;p;ca", ";state;p;us", "root;masa;t;ca", "root;hydra;z;kr", "envy;violet;z;kr", "eg;xenocider;t;us", "col;qxc;t;us", ";kane;z;ca",
			"eg;suppy;z;us", "f3;bails;p;us", "f3;puck;p;us", ";guitarcheese;z;us", "cm storm;polt;t;kr", "grav;semper;t;ca", "acer;scarlett;z;ca", "ta;astrea;p;us" };
		for (uint i = 0; i < samquals.size(); i++) {
			Player *p = &SearchPlayer(samquals[i] DEBUGARGS);
			p->league = chal.t_id;
			amquals[i] = p;
			frees.try_remove_vec(frees.northamericans, p);//need to use try because idle players...
			chal.ro64.matches[(i / 2) + 14].players[i % 2] = p;
		}
		array<const char*, 8> scnquals = { "ig;iasonu;z;cn", "zoo;toodming;z;cn", "ig;xigua;z;cn", "ig;macsed;p;cn",
			"ig;xy;t;cn", "ig;jim;p;cn", ";clavie;p;cn", "newbee;top 310;p;cn"
		};
		for (uint i = 0; i < scnquals.size(); i++) {
			Player *p = &SearchPlayer(scnquals[i] DEBUGARGS);
			p->league = chal.t_id;
			cnquals[i] = p;
			frees.try_remove_vec(frees.chinese, p);
			chal.ro64.matches[(i % 4) + 22].players[i / 4] = p;
		}
		array<const char*, 4> slaquals = { ";major;t;mx", "endurance;jimrising;z;mx", "root;catz;z;pe", "root;kelazhur;t;br" };
		for (uint i = 0; i < slaquals.size(); i++) {
			Player *p = &SearchPlayer(slaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			laquals[i] = p;
			frees.remove_vec(frees.latinamericans, p);
			chal.ro64.matches[(i / 2) + 26].players[i % 2] = p;
		}
		array<const char*, 4> sseaquals = { ";iaguz;t;au", "x5;pig;z;au", "psistm;enderr;z;ph", "x5;kingkong;z;kr", };
		for (uint i = 0; i < sseaquals.size(); i++) {
			Player *p = &SearchPlayer(sseaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			seaquals[i] = p;
			frees.remove_vec(frees.seas, p);
			chal.ro64.matches[(i % 2) + 28].players[i / 2] = p;
		}
		array<const char*, 4> stwquals = { "yoe;ian;z;tw", "hka;sen;z;tw", "yoe;has;p;tw", "wayi;rex;z;tw" };
		for (uint i = 0; i < stwquals.size(); i++) {
			Player *p = &SearchPlayer(stwquals[i] DEBUGARGS);
			p->league = chal.t_id;
			twquals[i] = p;
			frees.remove_vec(frees.taiwanese, p);
			chal.ro64.matches[(i / 2) + 30].players[i % 2] = p;
		}

		vector<string> urls;
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=6");//eu
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=7");//eu
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=8");//eu
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=12");//am
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=13");//am
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=16");//cn
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=19");//la
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=22");//sea
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_1/Challenger&action=edit&section=25");//tw
		vector<string> ress = HttpRequests(urls, true, false);
		for (auto s : ress) {
			vector<ScrapedGroup> smatches;
			ScrapeLiquipediaMatches(s, chal.t_id, smatches);
			for (uint i = 0; i < smatches.size(); i++) {
				auto *m = chal.ro64.GetMatch(smatches[i].players);
				if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chal.ro64.RoundOf());
				assert(m != NULL);
			}
		}

		prem.init_s1(prev_matches, upcoming_matches DEBUGARGS);

		/*for (auto &m : codes.ro32.matches) {
		for (auto *p : m.players) {
		//cerr << "removing free for "<<p->name.ToString()<<"\n";
		frees.remove_vec(frees.koreans, p);//replicated in init_s1
		//KRCtoP.AddAdvancingPlayer(p, 1,1,1,1,1,rng);
		}
		}*/
	}

	//void bake(Simulation &sim, FreePlayers &frees, WCSSeasonAdvancements &prev_advs, WCSSeasonAdvancements &next_advs, RayRand64 &rng DEBUGARGSDEF)
	//{
	//}

	void predict(Simulation &sim, FreePlayers &frees, WCSSeasonAdvancements &prev_advs, WCSSeasonAdvancements &next_advs, RayRand64 &rng DEBUGARGSDEF)
	{
		//MixedAdvancement<24,24> KRQtoC;
		//RandomAdvancement<32,32> KRCtoP;
		RandomAdvancement<6, 6> finalists;
		array<Player*, 32> wcsQtoC;
		RandomAdvancement<32, 32> wcsCtoP;

		//codeb.predict(sim, KRQtoC, frees, rng DEBUGARGS);
		//codea.AcceptAdvancement(KRQtoC.advancing_players);
		//codea.AcceptPrevBottom24(prev_advs.KR_bottom24, rng);
		//codea_s1.predict(sim, rng DEBUGARGS);
		//codes.RandomizeRo32(rng DEBUGARG);
		/*for(auto &m : codes.ro32.matches) {
		for(auto *p : m.players) {
		//cerr << "removing free for "<<p->name.ToString()<<"\n";
		frees.remove_vec( frees.koreans, p );
		//KRCtoP.AddAdvancingPlayer(p, 1,1,1,1,1,rng);
		}
		}*/
		//codes.AcceptAdvancement(KRCtoP.advancing_players);
		//codes.AcceptPrevTop8(prev_advs.KR_top8, rng);
		//codes.ro32.AcceptAdvancements(KRCtoP.advancing_players DEBUGARGS);
		codes.predict_s1(sim, next_advs.KR_top8, next_advs.KR_bottom24, finalists, rng DEBUGARGS);

		/*RandomAdvancement<16, 0> ramquals;
		for (auto p : amquals) {
		ramquals.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		for (auto p : amquals2) {
		ramquals.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/
		/*RandomAdvancement<4, 0> rlaquals;
		for (auto p : laquals) {
		rlaquals.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/
		/*RandomAdvancement<4, 0> rtwquals;
		for (auto p : twquals) {
		rtwquals.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}*/

		//quals.predict(sim, frees, wcsQtoC, rng DEBUGARGS);
		for (uint i = 0; i < euquals.size() / 2; i++) {
			//frees.add_vec(frees.europeans, wcsQtoC[i]);
			wcsQtoC[i] = euquals[i * 2];//0 to 13 of wcsQtoC is EU
		}
		for (uint i = 0; i < amquals.size() / 2; i++) {
			//frees.add_vec(frees.northamericans, wcsQtoC[i + 14]);
			wcsQtoC[i + 14] = amquals[i * 2];//14 to 21 is AM
		}
		for (uint i = 0; i < 4; i++) {
			//frees.add_vec(frees.chinese, wcsQtoC[i + 22]);
			wcsQtoC[i + 22] = cnquals[i];//22 to 26 is CN
		}
		for (uint i = 0; i < laquals.size() / 2; i++) {
			//frees.add_vec(frees.latinamericans, wcsQtoC[i + 26]);
			wcsQtoC[i + 26] = laquals[i * 2];//26 and 27 is LA
		}
		for (uint i = 0; i < 2; i++) {
			//frees.add_vec(frees.seas, wcsQtoC[i + 28]);
			wcsQtoC[i + 28] = seaquals[i];//28 and 29 is SEA
		}
		for (uint i = 0; i < 2; i++) {
			//frees.add_vec(frees.taiwanese, wcsQtoC[i + 30]);
			wcsQtoC[i + 30] = twquals[i * 2];
		}
		for (uint i = 0; i < wcsQtoC.size(); i++) {
			prev_advs.CtoC[i] = wcsQtoC[i];
			wcsQtoC[i] = NULL;
		}

		//quals.predict(sim, frees, wcsQtoC, rng DEBUGARGS);
		for (uint i = 0; i < euquals.size() / 2; i++) {
			//frees.add_vec(frees.europeans, wcsQtoC[i]);
			wcsQtoC[i] = euquals[i * 2 + 1];//0 to 13 of wcsQtoC is EU
		}
		for (uint i = 0; i < amquals.size() / 2; i++) {
			//frees.add_vec(frees.northamericans, wcsQtoC[i+14]);
			wcsQtoC[i + 14] = amquals[i * 2 + 1];//14 to 21 is AM
		}
		for (uint i = 0; i < 4; i++) {
			//frees.add_vec(frees.chinese, wcsQtoC[i + 22]);
			wcsQtoC[i + 22] = cnquals[i + 4];//22 to 26 is CN
		}
		for (uint i = 0; i < laquals.size() / 2; i++) {
			//frees.add_vec(frees.latinamericans, wcsQtoC[i + 26]);
			wcsQtoC[i + 26] = laquals[i * 2 + 1];//26 and 27 is LA
		}
		for (uint i = 0; i < 2; i++) {
			//frees.add_vec(frees.seas, wcsQtoC[i + 28]);
			wcsQtoC[i + 28] = seaquals[i + 2];//28 and 29 is SEA
		}
		for (uint i = 0; i < 2; i++) {
			//frees.add_vec(frees.taiwanese, wcsQtoC[i + 30]);
			wcsQtoC[i + 30] = twquals[i * 2 + 1];
		}
		/*for (auto p : prev_advs.CtoC) {
		sim.AddEvent(quals.t_id, p->player_id, 1);
		}
		for (auto p : wcsQtoC) {
		sim.AddEvent(quals.t_id, p->player_id, 1);
		}*/
		//chal.AcceptAdvancements(prev_advs.CtoC, wcsQtoC, prev_advs.champ, frees DEBUGARGS);
		chal.predict(sim, frees, next_advs.CtoC, wcsCtoP, rng DEBUGARGS);
		/*for(uint i=0;i<wcsQtoC.size();i++) {
		auto *p = wcsQtoC[i];
		wcsCtoP.AddAdvancingPlayer(p, 1,1,1,1,1,rng);
		next_advs.CtoC[i]=p;
		}*/
		//prem.AcceptAdvancements(wcsCtoP DEBUGARGS);
		prem.predict_s1(sim, rng, next_advs.champ DEBUGARGS);

		//spotv.AcceptAdvancement(prev_advs.SPOTV_top4, prev_advs.SPOTV_chalseeds DEBUGARGS);
		spotv.set_predict(sim, next_advs.SPOTV_top4, next_advs.SPOTV_chalseeds, rng DEBUGARGS);
	}
};

class WCSS2 : public WCSBase
{
public:
	SPOTVS2 spotv;

	//array<Player*, 24> codea_quals;
	array<Player*, 4> cnquals;
	array<Player*, 14> euquals;
	array<Player*, 2> seaquals;
	array<Player*, 7> amquals;
	array<Player*, 2> twquals;
	array<Player*, 2> laquals;

	array<Player*, 28> euchals;
	array<Player*, 16> amchals;
	array<Player*, 8> cnchals;
	array<Player*, 4> seachals;
	array<Player*, 4> twchals;
	array<Player*, 4> lachals;

	WCSS2() : WCSBase(2), spotv("SSL S2", (2 - 1) * 12 + 7)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees DEBUGARGSDEF)
	{
		WCSBase::init(prev_matches, upcoming_matches, frees DEBUGARGS);
		quals.init_s2(DEBUGARG);
		codea.matches.match_placing_to_tournament_placing[0] = 1;
		codea.init_s2(prev_matches, upcoming_matches, frees DEBUGARGS);
		codes.init_s2(prev_matches, upcoming_matches, frees DEBUGARGS);
		spotv.init_s2(prev_matches, upcoming_matches DEBUGARGS);

		array<const char*, 4> scnquals = {
			"bheart;mystery;p;cn", "bheart;cyan;p;cn", "x-team;courage;z;cn", "zoo;toodming;z;cn"
		};
		for (uint i = 0; i < scnquals.size(); i++) {
			Player *p = &SearchPlayer(scnquals[i] DEBUGARGS);
			p->league = chal.t_id;
			cnquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 14> seuquals = {
			"planetkey;gungfubanda;p;de", "mouz;heromarine;t;de", "tes;beastyqt;t;rs", "acer;bly;z;ua",
			"acer;nerchio;z;pl", ";elfi;p;fi", "millenium;marinelord;t;fr", "liquid;ret;z;nl",
			"millenium;lilbow;p;fr", "endurance;soul;t;pl", "property;zanster;z;se", "tes;elazer;z;pl",
			";petraeus;z;nz", "invasion;harstem;p;nl"
		};
		for (uint i = 0; i < seuquals.size(); i++) {
			Player *p = &SearchPlayer(seuquals[i] DEBUGARGS);
			p->league = chal.t_id;
			euquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 2> sseaquals = {
			"myi;blysk;p;sg", "silicon;nxz;z;au"
		};
		for (uint i = 0; i < sseaquals.size(); i++) {
			Player *p = &SearchPlayer(sseaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			seaquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 7> samquals = {
			"eg;jaedong;z;kr", "eg;xenocider;t;us", ";zeal 6300;p;us", "col;qxc;t;us", ";pilipili;p;kz", ";stardust;p;kr", "root;masa;t;ca"
		};
		for (uint i = 0; i < samquals.size(); i++) {
			Player *p = &SearchPlayer(samquals[i] DEBUGARGS);
			p->league = chal.t_id;
			amquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 2> stwquals = {
			";slam;z;tw", "wayi;cheetos;z;tw"
		};
		for (uint i = 0; i < stwquals.size(); i++) {
			Player *p = &SearchPlayer(stwquals[i] DEBUGARGS);
			p->league = chal.t_id;
			twquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 2> slaquals = {
			"root;catz;z;pe", "endurance;cham;z;mx"
		};
		for (uint i = 0; i < slaquals.size(); i++) {
			Player *p = &SearchPlayer(slaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			laquals[i] = p;
			frees.remove(p);
		}
		array<const char*, 28> seuchal = {
			"liquid;bunny;t;dk", "tes;beastyqt;t;rs", ";showtime;p;de", "tes;elazer;z;pl", "millenium;forgg;t;kr", "endurance;soul;t;pl", "liquid;snute;z;no", "acer;bly;z;ua",
			"liquid;tlo;z;de", "acer;nerchio;z;pl", "myi;serral;z;fi", "invasion;harstem;p;nl", ";firecake;z;fr", "property;zanster;z;se", ";welmu;p;fi", "liquid;ret;z;nl",
			"liquid;mana;p;pl", "mouz;heromarine;t;de", "at;uthermal;t;nl", "millenium;lilbow;p;fr", "property;morrow;t;se", ";petraeus;z;nz", ";happy;t;ru", "millenium;marinelord;t;fr",
			"property;naniwa;p;se", "planetkey;gungfubanda;p;de", ";targa;z;no", ";elfi;p;fi"
		};
		for (uint i = 0; i < seuchal.size(); i++) {
			Player *p = &SearchPlayer(seuchal[i] DEBUGARGS);
			p->league = chal.t_id;
			euchals[i] = p;
			chal.ro64.matches[i / 2].players[i % 2] = euchals[i];
			//if (i % 2 == 1) chal.ro64.matches[i / 2].EasyAddUpcoming(upcoming_matches, 0, chal.t_id, 64);
		}
		array<const char*, 16> samchal = {
			"root;hydra;z;kr", ";zeal 6300;p;us", ";kane;z;ca", ";pilipili;p;kz", "f3;puck;p;us", "eg;xenocider;t;us",
			"envy;violet;z;kr", "col;qxc;t;us", "eg;huk;p;ca", ";stardust;p;kr", "eg;suppy;z;us", "eg;jaedong;z;kr", "ta;astrea;p;us", "root;masa;t;ca",
			"cm storm;polt;t;kr", ";walkover"//polt gets walkover
		};
		for (uint i = 0; i < samchal.size(); i++) {
			Player *p = &SearchPlayer(samchal[i] DEBUGARGS);
			p->league = chal.t_id;
			amchals[i] = p;
			chal.ro64.matches[14 + i / 2].players[i % 2] = amchals[i];
			//if (i % 2 == 1 && i<15) chal.ro64.matches[14 + i / 2].EasyAddUpcoming(upcoming_matches, 0, chal.t_id, 64);
		}
		array<const char*, 8> scnchal = {
			"ig;iasonu;z;cn", "x-team;courage;z;cn", "ig;jim;p;cn", "bheart;cyan;p;cn",
			"ig;xigua;z;cn", "zoo;toodming;z;cn", "ig;macsed;p;cn", "bheart;mystery;p;cn"
		};
		for (uint i = 0; i < scnchal.size(); i++) {
			Player *p = &SearchPlayer(scnchal[i] DEBUGARGS);
			p->league = chal.t_id;
			cnchals[i] = p;
			chal.ro64.matches[22 + i / 2].players[i % 2] = cnchals[i];
		}
		array<const char*, 4> sseachal = {
			"x5;pig;z;au", "silicon;nxz;z;au", ";iaguz;t;au", "myi;blysk;p;sg"
		};
		for (uint i = 0; i < sseachal.size(); i++) {
			Player *p = &SearchPlayer(sseachal[i] DEBUGARGS);
			p->league = chal.t_id;
			seachals[i] = p;
			chal.ro64.matches[28 + i / 2].players[i % 2] = seachals[i];
		}
		array<const char*, 4> stwchal = {
			"hka;sen;z;tw", ";slam;z;tw", "yoe;has;p;tw", "wayi;cheetos;z;tw"
		};
		for (uint i = 0; i < stwchal.size(); i++) {
			Player *p = &SearchPlayer(stwchal[i] DEBUGARGS);
			p->league = chal.t_id;
			twchals[i] = p;
			chal.ro64.matches[30 + i / 2].players[i % 2] = twchals[i];
		}
		array<const char*, 4> slachal = {
			"root;kelazhur;t;br", "root;catz;z;pe", ";major juan;t;mx", "endurance;cham;z;mx"
		};
		for (uint i = 0; i < slachal.size(); i++) {
			Player *p = &SearchPlayer(slachal[i] DEBUGARGS);
			p->league = chal.t_id;
			lachals[i] = p;
			chal.ro64.matches[26 + i / 2].players[i % 2] = lachals[i];
		}

		vector<string> urls;
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=16");
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=22");
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=6");//EU
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=13");//AM
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=27");//TW
		urls.push_back("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_2/Challenger&action=edit&section=21");//LA
		vector<string> ress = HttpRequests(urls, true, false);
		for (auto s : ress) {
			vector<ScrapedGroup> smatches;
			ScrapeLiquipediaMatches(s, chal.t_id, smatches);
			for (uint i = 0; i < smatches.size(); i++) {
				auto *m = chal.ro64.GetMatch(smatches[i].players);
				if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chal.ro64.RoundOf());
				assert(m != NULL);
			}
		}

		prem.init_s2(prev_matches, upcoming_matches DEBUGARGS);
	}

	void predict(Simulation &sim, FreePlayers &frees, WCSSeasonAdvancements &prev_advs, WCSSeasonAdvancements &next_advs, RayRand64 &rng DEBUGARGSDEF)
	{
		RandomAdvancement<24, 24> KRCtoP;
		RandomAdvancement<6, 6> finalists;
		array<Player*, 32> wcsQtoC;
		RandomAdvancement<32, 32> wcsCtoP;

		codea.predict(sim, KRCtoP, frees, rng DEBUGARGS);
		//codes.AcceptAdvancement(KRCtoP.advancing_players);
		//codes.AcceptPrevTop8(prev_advs.KR_top8, rng);
		codes.predict_s2(sim, next_advs.KR_top8, next_advs.KR_bottom24, next_advs.gsl_finalists, rng DEBUGARGS);
		for (auto p : next_advs.gsl_finalists) finalists.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);

		//quals.predict(sim, frees, wcsQtoC, rng DEBUGARGS);
		for (auto &p : wcsQtoC) p = NULL;
		for (uint i = 0; i < 4; i++) {
			//frees.add_vec(frees.chinese, wcsQtoC[i + 22]);
			wcsQtoC[i + 22] = cnquals[i];//22 to 25 is CN
		}
		for (uint i = 0; i < euquals.size(); i++) {
			//frees.add_vec(frees.europeans, wcsQtoC[i]);
			wcsQtoC[i] = euquals[i];//0 to 13 of wcsQtoC is EU
		}
		RandomAdvancement<14, 0> reuquals;
		for (uint i = 0; i < 14; i++) reuquals.AddAdvancingPlayer(wcsQtoC[i], 0, 0, 0, 0, 0, rng);
		for (uint i = 0; i < 14; i++) wcsQtoC[i] = reuquals.advancing_players[i];

		for (uint i = 0; i < amquals.size(); i++) {
			//frees.add_vec(frees.northamericans, wcsQtoC[i + 14]);
			wcsQtoC[i + 14] = amquals[i];//14 to 21 is AM
		}
		RandomAdvancement<7, 0> ramquals;
		for (uint i = 14; i < 21; i++) ramquals.AddAdvancingPlayer(wcsQtoC[i], 0, 0, 0, 0, 0, rng);
		for (uint i = 14; i < 21; i++) wcsQtoC[i] = ramquals.advancing_players[i - 14];
		//frees.add_vec(frees.northamericans, wcsQtoC[21]);
		wcsQtoC[21] = &players[0];//walkover for Polt

		for (uint i = 0; i < 2; i++) {
			//frees.add_vec(frees.seas, wcsQtoC[i + 28]);
			wcsQtoC[i + 28] = seaquals[i];//28 and 29 is SEA
		}
		for (uint i = 0; i < twquals.size(); i++) {
			//frees.add_vec(frees.taiwanese, wcsQtoC[i + 30]);
			wcsQtoC[i + 30] = twquals[i];
		}
		RandomAdvancement<2, 0> rtwquals;
		for (uint i = 0; i < 2; i++) rtwquals.AddAdvancingPlayer(wcsQtoC[i + 30], 0, 0, 0, 0, 0, rng);
		for (uint i = 0; i < 2; i++) wcsQtoC[i + 30] = rtwquals.advancing_players[i];
		
		for (uint i = 0; i < laquals.size(); i++) {
			wcsQtoC[i + 26] = laquals[i];
		}
		RandomAdvancement<2, 0> rlaquals;
		for (uint i = 0; i < 2; i++) rlaquals.AddAdvancingPlayer(wcsQtoC[i + 26], 0, 0, 0, 0, 0, rng);
		for (uint i = 0; i < 2; i++) wcsQtoC[i + 26] = rlaquals.advancing_players[i];
		//for (uint i = 0; i <14; i++) {
		//sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);
		//}
		//for (uint i = 14; i <21; i++) {
		//sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);
		//}
		//for (uint i = 30; i <32; i++) {
		//	sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);
		//}
		chal.AcceptAdvancements(prev_advs.CtoC, wcsQtoC, prev_advs.champ, frees, rng DEBUGARGS);
		for (uint i = 0; i < 28; i++) {
			chal.ro64.matches[i / 2].players[i % 2] = euchals[i];
		}
		for (uint i = 0; i < 16; i++) {
			chal.ro64.matches[14 + i / 2].players[i % 2] = amchals[i];
		}
		for (uint i = 0; i < 8; i++) {
			chal.ro64.matches[22 + i / 2].players[i % 2] = cnchals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[26 + i / 2].players[i % 2] = lachals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[28 + i / 2].players[i % 2] = seachals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[30 + i / 2].players[i % 2] = twchals[i];
		}
		chal.predict(sim, frees, next_advs.CtoC, wcsCtoP, rng DEBUGARGS);
		//prem.AcceptAdvancements(wcsCtoP DEBUGARGS);
		prem.predict_s2(sim, rng, next_advs.champ DEBUGARGS);

		//spotv.AcceptAdvancement(prev_advs.SPOTV_top4, prev_advs.SPOTV_chalseeds DEBUGARGS);
		spotv.predict(sim, next_advs.SPOTV_top4, next_advs.SPOTV_chalseeds, rng DEBUGARGS);
	}
};

class WCSS3 : public WCSBase
{
public:
	SPOTVS3 spotv;
	array<Player*, 4> cnquals;
	array<Player*, 8> cnchals;
	array<Player*, 7> amquals;
	array<Player*, 14> euquals;
	array<Player*, 16> amchals;
	array<Player*, 28> euchals;
	array<Player*, 2> laquals;
	array<Player*, 4> lachals;
	array<Player*, 2> seaquals;
	array<Player*, 4> seachals;
	array<Player*, 2> twquals;
	array<Player*, 4> twchals;
	array<Player*, 24> gsl_quals;

	WCSS3() : WCSBase(3), spotv("SSL S3", (3 - 1) * 12 + 7)
	{
	}

	void init(vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, FreePlayers &frees, WCSSeasonAdvancements &prev_advs DEBUGARGSDEF)
	{
		WCSBase::init(prev_matches, upcoming_matches, frees DEBUGARGS);
		codea.init_s3(prev_matches, upcoming_matches, frees DEBUGARGS);
		codes.init_s3(prev_matches, upcoming_matches, frees DEBUGARGS);
		spotv.init_s3(prev_matches, upcoming_matches DEBUGARGS);
		quals.init_s3(DEBUGARG);

		array<const char*, 4> scnquals = {
			"x-team;shana;t;cn", "ig;xigua;z;cn", "x-team;courage;z;cn", "ig;xy;t;cn"
		};
		for (uint i = 0; i < scnquals.size(); i++) {
			Player *p = &SearchPlayer(scnquals[i] DEBUGARGS);
			p->league = chal.t_id;
			cnquals[i] = p;
			frees.remove(p);//cannot do this here until S2 is baked in
		}
		array<const char*, 8> scnchal = {//put the winner in the first slot so I can easily set the results
			"ig;xigua;z;cn", "zoo;toodming;z;cn", "ig;jim;p;cn", "x-team;courage;z;cn",
			"ig;iasonu;z;cn", "ig;xy;t;cn", "x-team;shana;t;cn", "bheart;mystery;p;cn"
		};
		for (uint i = 0; i < scnchal.size(); i++) {
			Player *p = &SearchPlayer(scnchal[i] DEBUGARGS);
			p->league = chal.t_id;
			cnchals[i] = p;
			chal.ro64.matches[22 + i / 2].players[i % 2] = cnchals[i];
			chal.ro64.matches[22 + i / 2].old_results.resize(1);
			chal.ro64.matches[22 + i / 2].old_results[0]=1;
			chal.ro64.matches[22 + i / 2].completion = 255;
		}
		array<const char*, 7> samquals = { "f3;bails;p;us", "f3;jonsnow;z;us", ";neeb;p;us", "col;qxc;t;us", ";hitman;p;us", "grav;semper;t;ca", "eg;huk;p;ca" };
		for (uint i = 0; i < samquals.size(); i++) {
			Player *p = &SearchPlayer(samquals[i] DEBUGARGS);
			p->league = chal.t_id;
			amquals[i] = p;
			frees.remove(p);//cannot do this here until S2 is baked in
		}
		array<const char*, 16> samchals = { "cm storm;polt;t;kr", "f3;jonsnow;z;us", "eg;jaedong;z;kr", "grav;semper;t;ca", ";stardust;p;kr", ";neeb;p;us",
			"envy;violet;z;kr", "col;qxc;t;us", "root;masa;t;ca", "f3;bails;p;us", "eg;xenocider;t;us", ";hitman;p;us", ";kane;z;ca", "eg;huk;p;ca",
			"root;hydra;z;kr", ";walkover" };
		for (uint i = 0; i < samchals.size(); i++) {
			auto p = &SearchPlayer(samchals[i] DEBUGARGS);
			p->league = chal.t_id;
			amchals[i] = p;
			chal.ro64.matches[14 + i / 2].players[i % 2] = amchals[i];
			//if (i % 2 == 1 && i<14) chal.ro64.matches[14 + i / 2].EasyAddUpcoming(upcoming_matches, Timestamp("July 22, 2015 18:00 {{Abbr /CDT"), chal.t_id, chal.ro64.RoundOf(), true);
		}
		array<const char*, 28> seuchals = { "millenium;lilbow;p;fr", ";sortof;z;se", "liquid;bunny;t;dk", ";starbuck;z;si", "liquid;tlo;z;de", "acer;bly;z;ua",
			"millenium;marinelord;t;fr", "isimba;dmc;p;ru", ";firecake;z;fr", "property;zanster;z;se", "planetkey;gungfubanda;p;de", "aaa;dns;p;fr",
			"millenium;forgg;t;kr", "millenium;dayshi;t;fr", "invasion;harstem;p;nl", "acer;nerchio;z;pl", "liquid;ret;z;nl", "mouz;heromarine;t;de",
			";elfi;p;fi", "at;uthermal;t;nl", "tes;elazer;z;pl", ";welmu;p;fi", "liquid;snute;z;no", "myi;serral;z;fi",
			"liquid;mana;p;pl", ";happy;t;ru", ";showtime;p;de", "property;naniwa;p;se" };
		for (uint i = 0; i < seuchals.size(); i++) {
			auto p = &SearchPlayer(seuchals[i] DEBUGARGS);
			p->league = chal.t_id;
			euchals[i] = p;
			chal.ro64.matches[i / 2].players[i % 2] = euchals[i];
			//if (i % 2 == 1) chal.ro64.matches[i / 2].EasyAddUpcoming(upcoming_matches, Timestamp("July 27, 2015 10:00 {{Abbr /CDT"), chal.t_id, chal.ro64.RoundOf(), true);
		}
		array<const char*, 15> seuquals = { ";showtime;p;de", ";welmu;p;fi", ";sortof;z;se", "acer;bly;z;ua",
			"acer;nerchio;z;pl", ";happy;t;ru", "property;naniwa;p;se", "at;uthermal;t;nl",
			"myi;serral;z;fi", "property;zanster;z;se", "mouz;heromarine;t;de", "isimba;dmc;p;ru",
			";starbuck;z;si", "aaa;dns;p;fr", "millenium;dayshi;t;fr" };
		for (uint i = 0; i < seuquals.size(); i++) {
			Player *p = &SearchPlayer(seuquals[i] DEBUGARGS);
			p->league = chal.t_id;
			if (i == 0) {//fill in Petraeus's empty slot
				for (auto &s : prev_advs.CtoC) {
					if (s == NULL) s = p;
				}
			} else euquals[i-1] = p;
			frees.remove(p);//cannot do this here until S2 is baked in
		}
		array<const char*, 2> slaquals = {
			";reason;z;br", "endurance;jimrising;z;mx"
		};
		for (uint i = 0; i < slaquals.size(); i++) {
			Player *p = &SearchPlayer(slaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			laquals[i] = p;
			frees.remove(p);//cannot do this here until S2 is baked in
		}
		array<const char*, 4> slachals = {//put the winner in the first slot so I can easily set the results
			"root;kelazhur;t;br", ";reason;z;br", ";major;t;mx", "endurance;jimrising;z;mx"
		};
		for (uint i = 0; i < slachals.size(); i++) {
			Player *p = &SearchPlayer(slachals[i] DEBUGARGS);
			p->league = chal.t_id;
			lachals[i] = p;
			chal.ro64.matches[26 + i / 2].players[i % 2] = lachals[i];
			//chal.ro64.matches[26 + i / 2].old_results.resize(1);
			//chal.ro64.matches[26 + i / 2].old_results[0] = 1;
			//chal.ro64.matches[26 + i / 2].completion = 255;
		}
		array<const char*, 2> sseaquals = {
			";petraeus;z;nz", "myi;blysk;p;sg"
		};
		for (uint i = 0; i < sseaquals.size(); i++) {
			Player *p = &SearchPlayer(sseaquals[i] DEBUGARGS);
			p->league = chal.t_id;
			seaquals[i] = p;
			frees.remove_vec(frees.seas, p);//cannot do this here until S2 is baked in
		}
		array<const char*, 4> sseachals = {
			"silicon;nxz;z;au", ";petraeus;z;nz", ";iaguz;t;au", "myi;blysk;p;sg"
		};
		for (uint i = 0; i < sseachals.size(); i++) {
			Player *p = &SearchPlayer(sseachals[i] DEBUGARGS);
			p->league = chal.t_id;
			seachals[i] = p;
			chal.ro64.matches[28 + i / 2].players[i % 2] = seachals[i];
			//chal.ro64.matches[28 + i / 2].old_results[0] = 1;
		}
		array<const char*, 2> stwquals = {
			"wayi;cheetos;z;tw", "yoe;ian;z;tw"
		};
		for (uint i = 0; i < stwquals.size(); i++) {
			Player *p = &SearchPlayer(stwquals[i] DEBUGARGS);
			p->league = chal.t_id;
			twquals[i] = p;
			frees.remove_vec(frees.taiwanese, p);//cannot do this here until S2 is baked in
		}
		array<const char*, 4> stwchals = {
			"yoe;has;p;tw", "wayi;cheetos;z;tw", "hka;sen;z;tw", "yoe;ian;z;tw"
		};
		for (uint i = 0; i < stwchals.size(); i++) {
			Player *p = &SearchPlayer(stwchals[i] DEBUGARGS);
			p->league = chal.t_id;
			twchals[i] = p;
			chal.ro64.matches[30 + i / 2].players[i % 2] = p;
		}

		string res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=Copa_America_2015/Season_2&action=edit&section=11", true, false);//LA
		vector<ScrapedGroup> smatches;
		ScrapeLiquipediaBracketsSimple(res, chal.t_id, smatches);
		for (uint i = 6; i < 8; i++) {
			auto m = chal.ro64.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chal.ro64.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Challenger&action=edit&section=24", true, false);//SEA
		smatches.clear();
		ScrapeLiquipediaMatches(res, chal.t_id, smatches);
		for (uint i = 0; i < 2; i++) {
			smatches[i].timestamp = Timestamp("July 11, 2015 21:45 {{Abbr /CDT");
			auto m = chal.ro64.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chal.ro64.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Challenger&action=edit&section=27", true, false);//TW
		smatches.clear();
		ScrapeLiquipediaMatches(res, chal.t_id, smatches);
		for (uint i = 0; i < 2; i++) {
			//smatches[i].timestamp = Timestamp("July 19, 2015 7:00 {{Abbr /CDT");
			auto m = chal.ro64.GetMatch(smatches[i].players);
			if (m) m->ReadOldResults(smatches[i], prev_matches, upcoming_matches, chal.ro64.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Challenger&action=edit&section=6", true, false);//EU
		smatches.clear();
		ScrapeLiquipediaMatches(res, chal.t_id, smatches);
		for (auto sm : smatches) {
			auto m = chal.ro64.GetMatch(sm.players);
			if (m) m->ReadOldResults(sm, prev_matches, upcoming_matches, chal.ro64.RoundOf());
			assert(m != NULL);
		}

		res = HttpRequest("http://wiki.teamliquid.net/starcraft2/index.php?title=2015_WCS_Season_3/Challenger&action=edit&section=13", true, false);//AM
		smatches.clear();
		ScrapeLiquipediaMatches(res, chal.t_id, smatches);
		for (auto sm : smatches) {
			auto m = chal.ro64.GetMatch(sm.players);
			if (m) m->ReadOldResults(sm, prev_matches, upcoming_matches, chal.ro64.RoundOf());
			assert(m != NULL);
		}
		chal.ro64.matches[21].old_results.push_back(1);
		chal.ro64.matches[21].completion = 255;

		/*array<const char*, 24> sgsl_quals = {
			";first;p;kr", "kt;zest;p;kr", "kt;super;p;kr", "jin air;trap;p;kr", "jin air;pigbaby;p;kr",
			"samsung;dear;p;kr", "samsung;hurricane;p;kr", "samsung;stork;p;kr", "mvp;yonghwa;p;kr", "skt t1;classic;p;kr", "yoe;san;p;kr",
			"sbenu;alive;t;kr", "sbenu;hack;t;kr", "sbenu;jjakji;t;kr", "cj entus;skyhigh;t;kr", "jin air;cure;t;kr", "prime;keen;t;kr",
			";sleep;z;kr", "samsung;armani;z;kr", "mvp;losira;z;kr", "skt t1;impact;z;kr", "psistm;true;z;kr", "roccat;hyun;z;kr", "sbenu;leenock;z;kr"
		};
		for (uint i = 0; i < sgsl_quals.size(); i++) {
			Player *p = &SearchPlayer(sgsl_quals[i] DEBUGARGS);
			p->league = codea.t_id;
			frees.remove_vec(frees.koreans, p);
			gsl_quals[i] = p;
		}*/

		prem.init_s3(prev_matches, upcoming_matches DEBUGARGS);
	}

	void predict(Simulation &sim, FreePlayers &frees, WCSSeasonAdvancements &prev_advs, WCSSeasonAdvancements &next_advs, RayRand64 &rng DEBUGARGSDEF)
	{
		//WCSBase::predict(sim, frees, prev_advs, next_advs, rng DEBUGARGS);
		//spotv.AcceptAdvancement(prev_advs.SPOTV_top4, prev_advs.SPOTV_chalseeds DEBUGARGS);
		spotv.predict(sim, next_advs.SPOTV_top4, next_advs.SPOTV_chalseeds, rng DEBUGARGS);

		RandomAdvancement<24, 24> KRQtoC;
		RandomAdvancement<24, 24> KRCtoP;
		RandomAdvancement<6, 6> finalists;
		array<Player*, 32> wcsQtoC;
		RandomAdvancement<32, 32> wcsCtoP;

		//codeb.predict(sim, KRQtoC, frees, rng DEBUGARGS);
		/*for (auto p : gsl_quals) {
			//frees.remove_vec(frees.koreans, p);
			KRQtoC.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);
		}
		codea.AcceptAdvancement(KRQtoC.advancing_players);
		codea.AcceptPrevBottom24(prev_advs.KR_bottom24, rng);*/
		codea.predict(sim, KRCtoP, frees, rng DEBUGARGS);
		//codes.AcceptAdvancement(KRCtoP.advancing_players);
		//codes.AcceptPrevTop8(prev_advs.KR_top8, rng);
		codes.predict_s3(sim, next_advs.KR_top8, next_advs.KR_bottom24, next_advs.gsl_finalists, rng DEBUGARGS);
		//codes.predict(sim, next_advs.KR_top8, next_advs.KR_bottom24, next_advs.gsl_finalists, rng DEBUGARGS);
		for (auto p : next_advs.gsl_finalists) finalists.AddAdvancingPlayer(p, 1, 1, 1, 1, 1, rng);

		//quals.predict(sim, frees, wcsQtoC, rng DEBUGARGS);

		//0-13 is eu, 14-21 is am, 22-25 is cn, 26-27 is la, 28-29 is sea, 30-31 is tw
		RandomAdvancement<14, 0> reuquals;
		for (uint i = 0; i < euquals.size(); i++) {
			//frees.add_vec(frees.europeans, wcsQtoC[i]);
			wcsQtoC[i] = euquals[i];
			reuquals.AddAdvancingPlayer(wcsQtoC[i], 1, 1, 1, 1, 1, rng);
		}
		for (uint i = (uint)euquals.size(); i < 14; i++) {
			//sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);//0-13 is EU
			reuquals.AddAdvancingPlayer(wcsQtoC[i], 1, 1, 1, 1, 1, rng);
		}
		for (uint i = 0; i < 14; i++) wcsQtoC[i] = reuquals.advancing_players[i];

		RandomAdvancement<8, 0> ramquals;
		for (uint i = 0; i < amquals.size(); i++) {
			//frees.add_vec(frees.northamericans, wcsQtoC[i + 14]);
			wcsQtoC[i + 14] = amquals[i];
			ramquals.AddAdvancingPlayer(amquals[i], 1, 1, 1, 1, 1, rng);
		}
		for (uint i = 14+(uint)amquals.size(); i < 21; i++) {
			//sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);//14-21 is AM
			ramquals.AddAdvancingPlayer(wcsQtoC[i], 1, 1, 1, 1, 1, rng);
		}
		//frees.add_vec(frees.northamericans, wcsQtoC[21]);
		wcsQtoC[21] = &players[0];//walkover for Hydra
		ramquals.AddAdvancingPlayer(wcsQtoC[21], 1, 1, 1, 1, 1, rng);
		for (uint i = 14; i < 22; i++) wcsQtoC[i] = ramquals.advancing_players[i - 14];

		for (uint i = 0; i < 4; i++) {
			//frees.add_vec(frees.chinese, wcsQtoC[i + 22]);
		}
		for (uint i = 0; i < 4; i++) {
			//frees.remove_vec(frees.chinese, cnquals[i]);
			wcsQtoC[i + 22] = cnquals[i];//22 to 26 is CN
		}
		for (uint i = 26; i < 28; i++) {
			//frees.add_vec(frees.latinamericans, wcsQtoC[i]);
			wcsQtoC[i] = laquals[i - 26];
			//sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);
		}

		RandomAdvancement<2, 0> rseaquals;
		for (uint i = 0; i < seaquals.size(); i++) {
			//frees.add_vec(frees.seas, wcsQtoC[i + 28]);
			wcsQtoC[i + 28] = seaquals[i];
			rseaquals.AddAdvancingPlayer(wcsQtoC[i + 28], 1, 1, 1, 1, 1, rng);
		}
		for (uint i = 0; i < 2; i++) wcsQtoC[i+28] = rseaquals.advancing_players[i];

		RandomAdvancement<2, 0> rtwquals;
		for (uint i = 0; i < twquals.size(); i++) {
			//frees.add_vec(frees.taiwanese, wcsQtoC[i + 28]);
			wcsQtoC[i + 30] = twquals[i];
			rtwquals.AddAdvancingPlayer(wcsQtoC[i + 30], 1, 1, 1, 1, 1, rng);
		}
		for (uint i = 0; i < 2; i++) wcsQtoC[i + 30] = rtwquals.advancing_players[i];

		//-----------------------------
		/*for (uint i = 30; i < 32; i++) {
			sim.AddEvent(quals.t_id, wcsQtoC[i]->player_id, 1);
		}*/

		chal.AcceptAdvancements(prev_advs.CtoC, wcsQtoC, prev_advs.champ, frees, rng DEBUGARGS);
		for (uint i = 0; i < 16; i++) {
			chal.ro64.matches[14 + i / 2].players[i % 2] = amchals[i];
		}
		for (uint i = 0; i < 28; i++) {
			chal.ro64.matches[i / 2].players[i % 2] = euchals[i];
		}
		for (uint i = 0; i < 8; i++) {
			chal.ro64.matches[22 + i / 2].players[i % 2] = cnchals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[26 + i / 2].players[i % 2] = lachals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[28 + i / 2].players[i % 2] = seachals[i];
		}
		for (uint i = 0; i < 4; i++) {
			chal.ro64.matches[30 + i/2].players[i%2] = twchals[i];
		}
		chal.predict(sim, frees, next_advs.CtoC, wcsCtoP, rng DEBUGARGS);
		//prem.AcceptAdvancements(wcsCtoP DEBUGARGS);
		prem.predict_s3(sim, rng, next_advs.champ DEBUGARGS);
	}
};

vector<PreviousMatch> prev_matches;
vector<UpcomingMatch> upcoming_matches;

uint GetUPMID(uint t_id, uint p_id, uint round=0)
{
	uint ret = 0;
	for (auto u : upcoming_matches) {
		if (u.t_id != t_id) continue;
		if (round && u.round != round) continue;
		for (auto p : u.players) {
			if (p == p_id) ret = u.id;
		}
	}
	return ret;
}

const int NUM_HEADBANDS = 4;
class HeadbandProcessor
{
public:
	vector<Headband> headband_matches;

	void Proc()
	{
		uint hbs[4];
		if (NUM_HEADBANDS >= 4) {
			headband_matches.push_back(Headband(SearchPlayer("liquid;taeja;t;kr" DEBUGARGS).player_id, SearchPlayer("skt t1;innovation;t;kr" DEBUGARGS).player_id, 3, 0, 1024, 1415469600 + 0, 0));
			headband_matches.push_back(Headband(SearchPlayer("skt t1;classic;p;kr" DEBUGARGS).player_id, SearchPlayer("cj entus;hero;p;kr" DEBUGARGS).player_id, 4, 0, 1024, 1415469600 + 2, 0));
		}
		headband_matches.push_back( Headband( SearchPlayer("acer;mma;t;kr" DEBUGARGS).player_id, SearchPlayer("skt t1;classic;p;kr" DEBUGARGS).player_id, 2, 0, 1024, 1415469600+5, 0) );
		headband_matches.push_back( Headband( SearchPlayer("kt;life;z;kr" DEBUGARGS).player_id, SearchPlayer("acer;mma;t;kr" DEBUGARGS).player_id, 1, 2, 1024, 1415469600+10, 0) );

		hbs[0] = SearchPlayer("kt;life;z;kr" DEBUGARGS).player_id;
		hbs[1] = SearchPlayer("acer;mma;t;kr" DEBUGARGS).player_id;
		hbs[2] = SearchPlayer("liquid;taeja;t;kr" DEBUGARGS).player_id;
		hbs[3] = SearchPlayer("skt t1;classic;p;kr" DEBUGARGS).player_id;

		for(auto &pm : prev_matches) {
			int winner_hb = 0;
			int loser_hb = 0;
			for (int i = 0; i < NUM_HEADBANDS; i++) {
				if (hbs[i] == pm.winner_id) winner_hb = i + 1;
				if (hbs[i] == pm.loser_id) loser_hb = i + 1;
			}
			if (winner_hb + loser_hb > 1) {
				uint challenger_id = pm.winner_id;
				int challenger_hb = winner_hb;
				uint defender_id = pm.loser_id;
				int defender_hb = loser_hb;
				if ( (loser_hb > winner_hb || loser_hb==0) && winner_hb > 0) {
					challenger_id = pm.loser_id;
					challenger_hb = loser_hb;
					defender_id = pm.winner_id;
					defender_hb = winner_hb;
				}
				while (pm.timestamp <= headband_matches.back().played_time) pm.timestamp = headband_matches.back().played_time + 1;
				Headband hb(pm, challenger_id, defender_id, defender_hb, challenger_hb);
				headband_matches.push_back(hb);
				hbs[defender_hb - 1] = pm.winner_id;
				if (challenger_hb) hbs[challenger_hb - 1] = pm.loser_id;
				//cerr << players[pm.winner_id].name.ToString() << " now has the #" << defender_hb << " headband after facing " << players[pm.loser_id].name.ToString() << " who now has the #" << challenger_hb << " headband\n";
			}
		}
		for (uint i = 0; i < NUM_HEADBANDS; i++) {
			cerr << players[hbs[i]].name.ToString() << " now has the #" << ToString(i + 1) << " headband\n";
		}
	}

	void Save(MysqlCon &con DEBUGARGSDEF)
	{
		if (BENCHMARK) return;

		con.delete_query("delete from prev_matches" DEBUGARGS);
		string prev_matches_query = "insert into prev_matches (winner_id,loser_id,t_id,played_on,round,score_winner,score_loser) values ";
		string values="";
		uint last_timestamp = 0;
		for (auto p : prev_matches) {
			uint timestamp = p.timestamp;
			if (timestamp <= last_timestamp) timestamp = last_timestamp + 1;
			last_timestamp = timestamp;
			values += "(" + ToString(p.winner_id) + "," + ToString(p.loser_id) + "," + ToString(p.t_id) + ",from_unixtime(" + ToString(timestamp) + ")," + ToString(p.round) + "," + ToString(p.score_winner) + "," + ToString(p.score_loser) + "),";
			//if (p.timestamp == 0) continue;
			//cerr << players[p.winner_id].name.ToString() << " beat " << players[p.loser_id].name.ToString() << " in " << TournamentNames[p.t_id] << ", "<<p.score_winner<<" to "<<p.score_loser<<" in the round of "<<p.round<<"\n";
			//break;
		}
		if (values.length()) {
			values = values.substr(0, values.length() - 1);
			prev_matches_query += values;
			con.update(prev_matches_query.c_str() DEBUGARGS);
		}

		con.delete_query("delete from headbands" DEBUGARGS);
		string headbands_query = "insert into headbands (defender_id,challenger_id,headband,defended,played_time,t_id,current) values ";
		values="";
		uint currents[NUM_HEADBANDS];
		for (int i = 0; i < NUM_HEADBANDS; i++) currents[i] = 0;
		for (uint i = 0; i<headband_matches.size(); i++) {
			auto &h = headband_matches[i];
			currents[h.headband - 1] = i;
		}
		for(uint i=0;i<headband_matches.size();i++) {
			auto &h = headband_matches[i];
			int current=0;
			if (currents[h.headband - 1] == i) current = 1;
			values+="("+ToString(h.defender)+","+ToString(h.challenger)+","+ToString(h.headband)+","+ToString(h.defended)+",from_unixtime("+ToString(h.played_time)+"),"+ToString(h.t_id)+","+ToString(current)+"),";
		}
		if(values.length()) {
			values = values.substr(0, values.length() - 1);
			headbands_query += values;
			con.update(headbands_query.c_str() DEBUGARGS);
		}
	}
}
headband_proc;

void ProcPrevMatches()
{
	headband_proc.Proc();
}

class WCS2015 : public TournamentBase
{
public:
	FreePlayers frees;

	PlaceholderTournaments placeholders;
	//Hot6ixCup hot6;
	//IEMSanJose iemsj;
	//IEMTaipei iemtp;
	//IEMKatowice iemk;
	//IEMShenzhen iemsz;

	//DHTours dhtours;
	//DHValencia dhvalencia;
	//DreamhackBase dhstockholm;
	//DHStockholm dhstockholm;

	//CopenhagenSpring copenhagenspring;

	//Gfinity1 gfinity1;
	//Gfinity2 gfinity2;
	//Gfinity3 gfinity3;
	//Gfinity4 gfinity4;

	//HKET3 hket3;

	//KeSPACup1 kespa1;
	//KeSPACup2 kespa2;
	//KeSPACup3 kespa3;

	//WCSS1 season1;
	//WCSS2 season2;
	//WCSS3 season3;

	//ASUSSummer asussummer;
	//PlaceholderBase iemcologne;
	//MSI msi;
	//IEMGamescom iemgamescom;
	//ForGamers forgamers;

	//array<Player*,4> SPOTV_seeds;
	WCSSeasonAdvancements s1_advs;
	WCSSeasonAdvancements s2_advs;

	WCS2015() : TournamentBase("WCS 2015", 0)
	{
	}

	void init(DEBUGARGDEF)
	{
		auto s = GetMilliCount();
		WCSS1 season1;
		WCSS2 season2;
		WCSS3 season3;
		Hot6ixCup hot6;
		IEMSanJose iemsj;
		IEMTaipei iemtp;
		IEMKatowice iemk;
		IEMShenzhen iemsz;
		CopenhagenSpring copenhagenspring;
		Gfinity1 gfinity1;
		Gfinity2 gfinity2;
		HKET3 hket3;
		KeSPACup1 kespa1;
		KeSPACup2 kespa2;
		DHTours dhtours;
		DHValencia dhvalencia("DreamHack Valencia", 47);
		DHStockholm dhstockholm;
		ASUSSummer asussummer;
		IEMGamescom iemgamescom;
		MSI msi;
		ForGamers forgamers;

		frees.init();
		//SPOTV_seeds[0]=&SearchPlayer("ig;jim;p;cn" DEBUGARGS);
		//SPOTV_seeds[1]=&SearchPlayer("kt;life;z;kr" DEBUGARGS);
		//SPOTV_seeds[2]=&SearchPlayer("acer;scarlett;z;ca" DEBUGARGS);
		//SPOTV_seeds[3]=&players[0];

		placeholders.init(prev_matches, upcoming_matches DEBUGARGS);
		hot6.init(prev_matches, upcoming_matches DEBUGARGS);
		iemsj.init(prev_matches, upcoming_matches DEBUGARGS);
		iemtp.init(prev_matches, upcoming_matches DEBUGARGS);
		iemk.init(prev_matches, upcoming_matches DEBUGARGS);
		iemsz.init(prev_matches, upcoming_matches DEBUGARGS);

		dhtours.init(prev_matches, upcoming_matches DEBUGARGS);
		dhvalencia.init(prev_matches, upcoming_matches DEBUGARGS);
		dhstockholm.init(prev_matches, upcoming_matches DEBUGARGS);
		//dhstockholm.ro16.ResetPlacings();
		//dhstockholm.quarterfinals.ResetPlacings();
		//dhstockholm.semifinals.ResetPlacings();

		copenhagenspring.init(prev_matches, upcoming_matches DEBUGARGS);

		gfinity1.init(prev_matches, upcoming_matches DEBUGARGS);
		gfinity2.init(prev_matches, upcoming_matches DEBUGARGS);
		//gfinity3.init(prev_matches, upcoming_matches DEBUGARGS);
		//gfinity4.init(prev_matches, upcoming_matches DEBUGARGS);

		hket3.init(prev_matches, upcoming_matches DEBUGARGS);

		kespa1.init(prev_matches, upcoming_matches DEBUGARGS);
		kespa2.init(prev_matches, upcoming_matches DEBUGARGS);
		//kespa3.init(prev_matches, upcoming_matches DEBUGARGS);

		asussummer.init(prev_matches, upcoming_matches DEBUGARGS);
		iemgamescom.init(prev_matches, upcoming_matches DEBUGARGS);
		//iemgamescom.t_id = 60;//62 is IEM Cologne Open Bracket
		//TournamentNames[60] = "IEM Gamescom";
		msi.init(prev_matches, upcoming_matches DEBUGARGS);
		//msi.t_id = 61;
		//TournamentNames[61] = "MSI Masters";
		forgamers.init(prev_matches, upcoming_matches DEBUGARGS);

		//iemgamescon.init(prev_matches, upcoming_matches DEBUGARGS);
		//iemgamescon.t_id = 64;//63 is IEM Gamescon Open Bracket
		//TournamentNames[64] = "IEM Gamescon";

		Simulation ovensim;
		RayRand64 rng;
		hot6.predict(ovensim, rng DEBUGARGS);
		hot6.completion = 255;
		iemsj.predict(ovensim, rng DEBUGARGS);
		iemsj.completion = 255;
		iemtp.predict(ovensim, rng DEBUGARGS);
		iemtp.completion = 255;
		iemk.predict(ovensim, rng DEBUGARGS);
		iemk.completion = 255;
		iemsz.predict(ovensim, rng DEBUGARGS);
		iemsz.completion = 255;
		iemgamescom.predict(ovensim, rng DEBUGARGS);
		iemgamescom.completion = 255;

		gfinity1.predict(ovensim, rng DEBUGARGS);
		gfinity1.completion = 255;
		gfinity2.predict(ovensim, rng DEBUGARGS);
		gfinity2.completion = 255;

		dhtours.predict(ovensim, rng DEBUGARGS);
		dhtours.completion = 255;
		dhvalencia.predict(ovensim, rng DEBUGARGS);
		dhvalencia.completion = 255;
		dhstockholm.predict(ovensim, rng DEBUGARGS);
		dhstockholm.completion = 255;

		hket3.predict(ovensim, rng DEBUGARGS);
		hket3.completion = 255;

		copenhagenspring.predict(ovensim, rng DEBUGARGS);
		copenhagenspring.completion = 255;

		kespa1.predict(ovensim, rng DEBUGARGS);
		kespa1.completion = 255;
		kespa2.predict(ovensim, rng, s2_advs.gsl_finalists DEBUGARGS);
		kespa2.completion = 255;

		asussummer.predict(ovensim, rng DEBUGARGS);
		asussummer.completion = 255;
		
		msi.predict(ovensim, rng DEBUGARGS);
		msi.completion = 255;

		forgamers.predict(ovensim, rng DEBUGARGS);
		forgamers.completion = 255;

		WCSSeasonAdvancements s0_advs;
		Player *walkover = &players[0];
		for (auto &p : s0_advs.CtoC) p = walkover;
		for (auto &p : s0_advs.KR_bottom24) p = walkover;
		for (auto &p : s0_advs.KR_top8) p = walkover;
		for (auto &p : s0_advs.SPOTV_top4) p = walkover;
		//s0_advs.SPOTV_chalseeds = SPOTV_seeds;
		s0_advs.champ = NULL;

		season1.init(prev_matches, upcoming_matches, frees DEBUGARGS);
		season1.predict(ovensim, frees, s0_advs, s1_advs, rng DEBUGARGS);
		season1.completion = 255;

		season2.init(prev_matches, upcoming_matches, frees DEBUGARGS);
		season2.predict(ovensim, frees, s1_advs, s2_advs, rng DEBUGARGS);
		season2.completion = 255;
		//void Petraeus's seed
		for (auto &p : s2_advs.CtoC) {
			if (p->player_id == 747) {
				frees.add(p);
				p = NULL;
			}
		}

		season3.init(prev_matches, upcoming_matches, frees, s2_advs DEBUGARGS);

		ovensim.AddPlayerWCSPoints(players[575], 50);//give uThermal his 50 points for his challenger forfeit
		ovensim.AddPlayerWCSPoints(players[103], -50);//take 50 points away from elfi for his challenger loss
		ovensim.AddPlayerWCSPoints(players[5978], 150);//give Hitman his 150 points for his ro16 forfeit
		ovensim.AddPlayerWCSPoints(players[5847], -150);//take 150 points from Elazer for his ro32 loss

		WCSSeasonAdvancements s3_advs;
		season3.predict(ovensim, frees, s2_advs, s3_advs, rng DEBUGARGS);

		for (auto pp : ovensim.player_points) {
			uint pid = pp.first;
			uint points = pp.second;
			GetPlayerById(pid DEBUGARGS).wcs_points = points;
		}
		for (auto pp : ovensim.player_wcs_points) {
			uint pid = pp.first;
			uint points = pp.second;
			GetPlayerById(pid DEBUGARGS).wcs_wcs_points = points;
		}

		stable_sort(prev_matches.begin(), prev_matches.end(), [](const PreviousMatch &a, const PreviousMatch &b) {
			return a.timestamp<b.timestamp;
		});
		stable_sort(upcoming_matches.begin(), upcoming_matches.end(), [](const UpcomingMatch &a, const UpcomingMatch &b) {
			return a.timestamp<b.timestamp;
		});
		ProcPrevMatches();
		assert(upcoming_matches.size()<255);

		cerr << "\n\n";
		for (auto &u : upcoming_matches) {
			string against;
			for (uint i = 0; i<8; i++) {
				if (u.players[i]) {
					against += players[u.players[i]].name.ToString() + ", ";
				}
			}
			string hours = ToString(double(u.timestamp - (uint)time(0)) / 3600.0);
			cerr << "upcoming match " << u.id << " in t_id " << u.t_id << ", with " << against << " in " << hours << " hours\n";
		}

		auto took = GetMilliSpan(s);
		cerr << "\nInit took " << took << "ms\n";
	}

	void predict(Simulation &sim, RayRand64 &rng DEBUGARGSDEF)
	{
		auto tfrees = frees;
		for (auto &p : players) {
			if (p.wcs_wcs_points) {
				sim.AddPlayerWCSPoints(p, 0);
			}
			else if (p.wcs_points) {
				sim.AddPlayerPoints(p, 0);
			}
		}//will need to redo this to do wcs and non-wcs points separately, can just use a global/static vector

		//WCSSeasonAdvancements s2_advs;
		//WCSSeasonAdvancements s3_advs;

		placeholders.predict(sim, rng DEBUGARGS);

		//asussummer.predict(sim, rng DEBUGARGS);
		//iemgamescom.predict(sim, rng DEBUGARGS);
		//msi.predict(sim, rng DEBUGARGS);

		//dhtours.predict(sim, rng DEBUGARGS);
		//dhvalencia.predict(sim, rng DEBUGARGS);
		//dhstockholm.predict(sim, rng DEBUGARGS);

		//iemsz.predict(sim, rng DEBUGARGS);

		//gfinity2.predict(sim, rng DEBUGARGS);
		//gfinity3.predict(sim, rng DEBUGARGS);
		//gfinity4.predict(sim, rng DEBUGARGS);

		//hket3.predict(sim, rng DEBUGARGS);

		//kespa1.predict(sim, rng DEBUGARGS);

		//season2.predict(sim, tfrees, s1_advs, s2_advs, rng DEBUGARGS);

		//kespa2.predict(sim, rng, s2_advs.gsl_finalists DEBUGARGS);
		//forgamers.predict(sim, rng DEBUGARGS);

		//season3.predict(sim, tfrees, s2_advs, s3_advs, rng DEBUGARGS);
		/*array<Player*, 2> SSL_finalists;
		SSL_finalists[0] = s3_advs.SPOTV_top4[0];
		SSL_finalists[1] = s3_advs.SPOTV_top4[1];*/
		//kespa3.predict(sim, rng, s3_advs.gsl_finalists, SSL_finalists DEBUGARGS);
	}
};
WCS2015 wcs;