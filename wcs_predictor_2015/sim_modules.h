
class SimModule
{
public:
	virtual ~SimModule() {}

	virtual void Proc(vector<Simulation> &sims, array<uint,MAX_PLAYER_ID> &players_times_top) //vector< pair<uint,uint> > &players_times_top)
	{
	}

	virtual void Save(MysqlCon &con, size_t sim_id DEBUGARGSDEF)
	{
	}

	virtual void ClearCounters() = 0;
};


class ForeignHopeHandler : public SimModule
{
public:
	bool inited;
	array<uint, TOP_NUM_QUALIFY + 1> num_foreign_hopes;
	vector< pair<Player*, uint> > foreign_hopes;

	ForeignHopeHandler()
	{
		inited = false;
	}

	void TickAddOnlyForeigner(array<Player*, TOP_NUM_QUALIFY> top_players DEBUGARGSDEF)
	{
		Player *the_one = NULL;
		for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
			if (memcmp(top_players[i]->country, "kr", 2)) {
				the_one = top_players[i];
				break;
			}
		}
		assert(the_one != NULL);

		for (auto &i : foreign_hopes) {
			if (i.first == the_one) {
				i.second++;
				return;
			}
		}
		foreign_hopes.push_back(make_pair(the_one, 1u));
	}

	void TickOnlyForeigner(array<Player*, TOP_NUM_QUALIFY> top_players DEBUGARGSDEF)
	{
		Player *the_one = NULL;
		for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
			if (memcmp(top_players[i]->country, "kr", 2)) {
				the_one = top_players[i];
				break;
			}
		}
		assert(the_one != NULL);

		for (auto &i : foreign_hopes) {
			if (i.first == the_one) {
				i.second++;
				return;
			}
		}
	}

	void Print(vector<Simulation> &sims DEBUGARGSDEF)
	{
		cerr << "num foreigners\n";
		for (uint f = 0; f<num_foreign_hopes.size(); f++) {
			uint times = num_foreign_hopes[f];
			cerr << "there are " << f << " foreigners ~ " << ((double)times / (double)sims.size() *100.0) << " % of the time\n";
		}
		cerr << "foreign hopes\n";
		for (auto &f : foreign_hopes) {
			cerr << f.first->name << " is the only foreigner ~ " << ((double)f.second / (double)sims.size() *100.0) << " % of the time\n";
		}
	}

	virtual void ClearCounters()
	{
		//clear the old values, accumulate add in mysql so I can support easy clustering
		for (auto &f : num_foreign_hopes)
			f = 0;
		for (auto &f : foreign_hopes)
			f.second = 0;
	}

	void InitSeason(vector<Simulation> &sims DEBUGARGSDEF)
	{
		ClearCounters();

		for (uint sim = 0; sim<sims.size(); sim++) {
			auto &s = sims[sim];
			uint num_foreigners = 0;
			array<Player*, TOP_NUM_QUALIFY> top_players;
			for (uint i = 0; i<TOP_NUM_QUALIFY; i++)
				top_players[i] = &GetPlayerById(s.player_points[i].first DEBUGARGS);

			for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
				if (memcmp(top_players[i]->country, "kr", 2)) {
					num_foreigners++;
				}
			}
			num_foreign_hopes[num_foreigners]++;
			if (num_foreigners == 1) {
				TickAddOnlyForeigner(top_players DEBUGARGS);
			}
		}

		std::sort(foreign_hopes.begin(), foreign_hopes.end(), [](const pair<Player*, uint> a, const pair<Player*, uint> b) {
			return a.second>b.second;
		});
		if (foreign_hopes.size()>32) {
			foreign_hopes.resize(32);
		}

		//Print(sims, season DEBUGARGS);
	}

	void ProcessSeason(vector<Simulation> &sims DEBUGARGSDEF)
	{
		ClearCounters();

		for (uint sim = 0; sim<sims.size(); sim++) {
			auto &s = sims[sim];
			uint num_foreigners = 0;
			array<Player*, TOP_NUM_QUALIFY> top_players;
			for (uint i = 0; i<TOP_NUM_QUALIFY; i++)
				top_players[i] = &GetPlayerById(s.player_points[i].first DEBUGARGS);

			for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
				if (memcmp(top_players[i]->country, "kr", 2)) {
					num_foreigners++;
				}
			}
			num_foreign_hopes[num_foreigners]++;
			if (num_foreigners == 1) {
				TickOnlyForeigner(top_players DEBUGARGS);
			}
		}

		std::sort(foreign_hopes.begin(), foreign_hopes.end(), [](const pair<Player*, uint> a, const pair<Player*, uint> b) {
			return a.second>b.second;
		});

		//Print(sims, season DEBUGARGS);
	}

	virtual void Proc(vector<Simulation> &sims, array<uint, MAX_PLAYER_ID> &players_times_top)
	{
		//cerr << "Processing ForeignHopeHandler\n";
		if (inited == false) {
			InitSeason(sims DEBUGARGS);
		}
		else {
			ProcessSeason(sims DEBUGARGS);
		}
		//cerr << "Done Processing ForeignHopeHandler\n";
	}

	void Save(MysqlCon &con, unsigned __int64 sim_id DEBUGARGSDEF)
	{
		string queryA = "insert into num_foreigner_hopes (sim_id,season,num_foreigners,num_times) values ";
		string queryB = "insert into foreigner_hopes (sim_id,season,p_id,num_times_only) values ";
		string valuesA;
		string valuesB;

		for (uint f = 0; f<num_foreign_hopes.size(); f++) {
			valuesA += "(" + ToString(sim_id) + ",0," + ToString(f) + "," + ToString(num_foreign_hopes[f]) + "),";
		}
		for (auto f : foreign_hopes) {
			valuesB += "(" + ToString(sim_id) + ",0," + ToString(f.first->player_id) + "," + ToString(f.second) + "),";
		}

		if (valuesA.length()) {
			valuesA = valuesA.substr(0, valuesA.length() - 1);
			queryA += valuesA + " on duplicate key update num_times=num_times+values(num_times)";
			con.update(queryA.c_str() DEBUGARGS);
		}
		if (valuesB.length()) {
			valuesB = valuesB.substr(0, valuesB.length() - 1);
			queryB += valuesB + " on duplicate key update num_times_only=num_times_only+values(num_times_only)";
			con.update(queryB.c_str() DEBUGARGS);
		}
	}
};

class FinalFacingStat
{
public:
	uint p_id1;
	uint p_id2;
	uint hits;

	FinalFacingStat(uint PID1, uint PID2)
	{
		hits = 0;
		if (PID1<PID2) {
			p_id1 = PID1;
			p_id2 = PID2;
		}
		else if (PID2<PID1) {
			p_id1 = PID2;
			p_id2 = PID1;
		}
	}

	uint Hash()
	{
		uint h = p_id1^p_id2;
		h *= p_id1;
		h += p_id2;
		return h;
	}
};
class SeedChances
{
public:
	uint pid;
	uint hits;
	SeedChances()
	{
		pid = hits = 0;
	}

	uint Hash()
	{
		uint h = pid;
		h += pid ^ 547637;
		h -= pid / 7;
		return h;
	}
};
class FinalFacingStats : public SimModule
{
public:
	bool inited;
	array<vector<FinalFacingStat>, 1024> d;
	array<array<vector<SeedChances>, 1024>, TOP_NUM_QUALIFY> seeds;
	//vector<pair<string,uint> > brackets;

	FinalFacingStats()
	{
		inited = false;
	}

	SeedChances* GetSeed(SeedChances sc, uint seed)
	{
		uint h = sc.Hash() % (uint)seeds[seed].size();
		auto &s = seeds[seed][h];
		for (auto &p : s) {
			if (p.pid == sc.pid) return &p;
		}
		return NULL;
	}

	SeedChances* MakeSeed(SeedChances sc, uint seed)
	{
		SeedChances *ret = GetSeed(sc, seed);
		if (ret) return ret;
		uint h = sc.Hash() % (uint)seeds[seed].size();
		auto &s = seeds[seed][h];
		s.push_back(sc);
		return &s.back();
	}

	FinalFacingStat* GetStat(FinalFacingStat ffs)
	{
		uint h = ffs.Hash() % d.size();
		for (auto &f : d[h]) {
			if (f.p_id1 == ffs.p_id1 && f.p_id2 == ffs.p_id2) return &f;
		}
		return NULL;
		//d[h].push_back(ffs);
		//return d[h].back();
	}

	FinalFacingStat* MakeStat(FinalFacingStat ffs)
	{
		uint h = ffs.Hash() % d.size();
		for (auto &f : d[h]) {
			if (f.p_id1 == ffs.p_id1 && f.p_id2 == ffs.p_id2) return &f;
		}
		d[h].push_back(ffs);
		return &d[h].back();
	}

	void InitSim(Simulation &sim, double dsims, array<uint, MAX_PLAYER_ID> &players_times_top DEBUGARGSDEF)
	{
		uint A = 0;
		uint B = TOP_NUM_QUALIFY - 1;
		for (; A<B; A++, B--) {
			uint pid1 = sim.player_points[A].first;
			uint pid2 = sim.player_points[B].first;

			Player &player1 = players[pid1];
			Player &player2 = players[pid2];

			uint total_times_tops1 = players_times_top[player1.player_id];
			double prob1 = (double)total_times_tops1 / dsims;

			uint total_times_tops2 = players_times_top[player2.player_id];
			double prob2 = (double)total_times_tops2 / dsims;

			if (prob1>0.02 && prob2>0.02 /*&& (prob1>0.05 || prob2>0.05)*/) {
				FinalFacingStat f(pid1, pid2);
				auto *p = MakeStat(f);
				p->hits++;
			}
		}

		for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
			uint pid = sim.player_points[i].first;
			//Player &p = players[pid];
			uint total_times_tops = players_times_top[pid];
			double prob = (double)total_times_tops / dsims;

			if (prob>0.02) {
				SeedChances sc;
				sc.pid = pid;
				auto *r = MakeSeed(sc, i);
				r->hits++;
			}
		}
	}

	void CountBrackets(vector<Simulation> &sims DEBUGARGSDEF)
	{
		uint ASeedToMatch[8] = { 0, 7, 5, 3, 2, 4, 6, 1 };
		/*
		#1 vs #16
		#8 vs #9

		#5 vs #12
		#4 vs #13

		#6 vs #11
		#3 vs #14

		#7 vs #10
		#2 vs #15
		*/

		/*for (auto &sim : sims) {
		string bracket;
		string matches[8];
		uint A = 0;
		uint B = TOP_NUM_QUALIFY - 1;
		for (; A < B; A++, B--) {
		uint pid1 = sim.player_points[A].first;
		uint pid2 = sim.player_points[B].first;
		uint match = ASeedToMatch[A];
		matches[match] = "#" + ToString(A+1) + " " + players[pid1].name.ToString() + " vs #" + ToString(B+1) + " " + players[pid2].name.ToString();
		}
		for (uint i = 0; i < 8; i++) {
		bracket += matches[i] + "\n";
		if (i == 3) bracket += "\n";
		}
		bool found = false;
		for (uint i = 0; i < brackets.size(); i++) {
		if (brackets[i].first == bracket) {
		found = true;
		brackets[i].second++;
		break;
		}
		}
		if (found == false) {
		brackets.push_back(make_pair(bracket, 1u));
		}
		}*/
	}

	void InitSeason(vector<Simulation> &sims, array<uint, MAX_PLAYER_ID> &players_times_top DEBUGARGSDEF)
	{
		const uint skips = 32;
		double dsims = (double)sims.size();
		for (uint i = 0; i<sims.size(); i += skips) {
			InitSim(sims[i], dsims, players_times_top DEBUGARGS);
		}
		//remove anything with less than ~0.1% chances
		for (auto &h : d) {
			for (uint f = 0; f<h.size(); f++) {
				if (h[f].hits*skips * 1000 < sims.size()) {
					h[f] = h.back();
					h.pop_back();
					f--;//do this slot again since it's a new item
				}
			}
		}
		for (auto &d : seeds) {
			for (auto &h : d) {
				for (uint i = 0; i<h.size(); i++) {
					if (h[i].hits*skips * 1000 < sims.size()) {
						h[i] = h.back();
						h.pop_back();
						i--;//do this slot again since it's a new item
					}
				}
			}
		}
		ClearCounters();
		ProcessSeason(sims DEBUGARGS);

		/*CountBrackets(sims DEBUGARGS);
		sort(brackets.begin(), brackets.end(), [](const pair<string, uint> &A, const pair<string, uint> &B) {
		return A.second > B.second;
		});
		cout << "\n\n";
		cout << brackets.size() << " possible brackets\n";
		for (uint i = 0; i < brackets.size();i++) {
		double p = (double)brackets[i].second / (double)sims.size() *100.0;
		cout << "\n~ " << p << " % chance\n" << brackets[i].first << "------------------\n";
		}
		cout << "\n\n";
		brackets.clear();*/
		//exit(0);
	}

	void ProcSim(Simulation &sim DEBUGARGSDEF)
	{
		uint A = 0;
		uint B = TOP_NUM_QUALIFY - 1;
		for (; A<B; A++, B--) {
			uint pid1 = sim.player_points[A].first;
			uint pid2 = sim.player_points[B].first;

			FinalFacingStat f(pid1, pid2);
			auto *p = GetStat(f);
			if (p) p->hits++;
		}
		for (uint i = 0; i<TOP_NUM_QUALIFY; i++) {
			uint pid = sim.player_points[i].first;
			SeedChances sc;
			sc.pid = pid;
			auto *r = MakeSeed(sc, i);
			if (r) r->hits++;
		}
	}

	void ProcessSeason(vector<Simulation> &sims DEBUGARGSDEF)
	{
		for (uint i = 0; i<sims.size(); i++) {
			ProcSim(sims[i] DEBUGARGS);
		}
	}

	virtual void ClearCounters()
	{
		for (auto &h : d) {
			for (auto &f : h) {
				f.hits = 0;
			}
		}
		for (auto &d : seeds) {
			for (auto &h : d) {
				for (auto &s : h) {
					s.hits = 0;
				}
			}
		}
	}

	virtual void Proc(vector<Simulation> &sims, array<uint, MAX_PLAYER_ID> &players_times_top)
	{
		//return;
		//cerr << "Processing FinalFacingStats\n";
		ClearCounters();
		if (inited == false) {
			InitSeason(sims, players_times_top DEBUGARGS);
			inited = true;
		}
		else {
			ProcessSeason(sims DEBUGARGS);
		}
		//cerr << "Done Processing FinalFacingStats\n";
	}

	void Save(MysqlCon &con, unsigned __int64 sim_id DEBUGARGSDEF)
	{
		//return;
		//x-will need to run create table finalfacingstats(sim_id int unsigned not null, season int unsigned not null, pid1 int unsigned not null, pid2 int unsigned not null, num_times int unsigned not null, primary key(sim_id,season,pid1,pid2));
		string queryA = "insert into finalfacingstats (sim_id,season,pid1,pid2,num_times) values ";
		string valuesA;
		string queryB = "insert into finalseedstats (sim_id,season,pid,seed,num_times) values ";
		string valuesB;

		for (auto &h : d) {
			for (auto &f : h) {
				if (f.hits>0) {
					valuesA += "(" + ToString(sim_id) + ",0," + ToString(f.p_id1) + "," + ToString(f.p_id2) + "," + ToString(f.hits) + "),";
				}
			}
		}

		if (valuesA.length()) {
			valuesA = valuesA.substr(0, valuesA.length() - 1);
			queryA += valuesA + " on duplicate key update num_times=num_times+values(num_times)";
			con.update(queryA.c_str() DEBUGARGS);
		}

		//for(auto &d:seeds) {
		for (uint seed = 0; seed<TOP_NUM_QUALIFY; seed++) {
			auto &d = seeds[seed];
			for (auto &h : d) {
				for (auto &s : h) {
					if (s.hits>0) {
						valuesB += "(" + ToString(sim_id) + ",0," + ToString(s.pid) + "," + ToString(seed + 1) + "," + ToString(s.hits) + "),";
					}
				}
			}
		}
		if (valuesB.length()) {
			valuesB = valuesB.substr(0, valuesB.length() - 1);
			queryB += valuesB + " on duplicate key update num_times=num_times+values(num_times)";
			con.update(queryB.c_str() DEBUGARGS);
		}
	}
};
