
template<class T, size_t SLOTS>
class FakeVector
{
public:
	uint s;
	std::array<T, SLOTS> a;

	FakeVector()
	{
		s = 0;
	}

	auto begin() -> decltype(a.begin())
	{
		return a.begin();
	}

	auto end() -> decltype(a.begin())
	{
		return a.begin() + s;
	}

	void push_back(T d)
	{
		assert((size_t)s+10 < SLOTS);
		a[s] = d;
		s++;
	}

	size_t size() { return (size_t)s; }

	T& operator[](size_t i)
	{
		return a[i];
	}

	void clear()
	{
		s = 0;
	}

	T& back() { return a[s - 1]; }

	void shrink_to_fit() {}
	void reserve(size_t ns) {}
	size_t capacity() { return SLOTS; }
};

class Simulation
{
public:
	vector<pair<unsigned short, short> > player_points;
	//vector<pair<uint, int> > player_points_season[NUM_SEASONS];
	//FakeVector<Event, 150> events;
	//bitset<SIM_EVENTS_BLOOM_SIZE*64> bloomfilter[NUM_SEASONS+1];
	unsigned __int64 bloomfilter[SIM_EVENTS_BLOOM_SIZE];

	//FakeVector<Event, 145> events;
	vector<Event> events;
	vector<pair<unsigned short, short> > player_wcs_points;

	Simulation()
	{
		for(uint i=0;i<SIM_EVENTS_BLOOM_SIZE;i++)
			bloomfilter[i]=0;
	}

	void _AddPlayerPoints(Player &player, int points)
	{
		for (auto &pp : player_points) {
			if (pp.first == player.player_id) {
				pp.second += points;
				if (BDEBUG && pp.second<0) {
					DEBUGOUT(FATAL, player.name.ToString(), points);
				}
				return;
			}
		}
		if (BDEBUG && points < 0) {
			DEBUGOUT(FATAL, player.name.ToString(), points);
		}
		player_points.push_back(make_pair(player.player_id, player.wcs_points + points));
	}

	void _AddPlayerWCSPoints(Player &player, int points)
	{
		for (auto &pp : player_wcs_points) {
			if (pp.first == player.player_id) {
				pp.second += points;
				if (BDEBUG && pp.second<0) {
					DEBUGOUT(FATAL, player.name.ToString(), points);
				}
				return;
			}
		}
		if (BDEBUG && points < 0) {
			DEBUGOUT(FATAL, player.name.ToString(), points);
		}
		player_wcs_points.push_back(make_pair(player.player_id, player.wcs_wcs_points + points));
	}

	void AddPlayerPoints(Player &player, int points)
	{
		//if (points == 0) cerr << "wtf?\n";//return;
		/*if (BDEBUG && player.player_id == 300) {
			//cerr << "giving "<<player.name.ToString()<<" "<<points<<" points\n";
			DEBUGOUT(FATAL, points, player.name.ToString());
		}*/

		_AddPlayerPoints(player, points);
	}

	void AddPlayerWCSPoints(Player &player, int points)
	{
		/*if (BDEBUG && player.player_id == 189) {
			cerr << "giving " << player.name.ToString() << " " << points << " WCS points\n";
		}*/
		//return;
		//if (points == 0) cerr << "wtf?\n";//return;
		_AddPlayerWCSPoints(player, points);
		_AddPlayerPoints(player, points);
	}

	void AddEvent(uint t_id, uint p_id, uint placing)
	{
		//return;
		events.push_back(Event(t_id, p_id, placing));
		uint h = events.back().Hash() % (SIM_EVENTS_BLOOM_SIZE * 64);
		bloomfilter[h >> 6] |= (unsigned __int64)1 << ((unsigned __int64)h % 64);
	}

	void RecalcBloomFilter()
	{
		events.shrink_to_fit();
		for(uint i=0;i<SIM_EVENTS_BLOOM_SIZE;i++)
			bloomfilter[i]=0;
		for(auto &ev : events) {
			uint h = ev.Hash() % (SIM_EVENTS_BLOOM_SIZE * 64);
			bloomfilter[h >> 6] |= (unsigned __int64)1 << ((unsigned __int64)h % 64);
		}
	}

	const size_t FindEvents(EventGroup evg)
	{

		size_t s = evg.size();
		for (size_t e = 0; e < s; e++) {
			uint h = evg.events[e].Hash() % (SIM_EVENTS_BLOOM_SIZE * 64);
			if ((bloomfilter[h >> 6] & ((unsigned __int64)1 << ((unsigned __int64)h % 64))) == 0)
				//if(bloomfilter[season][h]==false)
				return 0;
		}
		for (size_t e = 0; e < s; e++) {
			if (_FindEvent(evg.events[e]) == false)
				return 0;
		}
		return s;
	}

	const bool _FindEvent(Event ev)
	{
		int imax = ((int)events.size()) - 1;
		int imin = 0;
		while (imax >= imin)
		{
			int imid = (imax - imin) / 2 + imin;
			int cmp = CompEvents(events[imid], ev);
			if (cmp == 1)
				imin = imid + 1;
			else if (cmp == -1)
				imax = imid - 1;
			else {
				return true;
			}
		}
		return false;
	}

	const bool FindEvent(Event ev)
	{
		uint h = ev.Hash() % (SIM_EVENTS_BLOOM_SIZE * 64);
		if ((bloomfilter[h >> 6] & ((unsigned __int64)1 << ((unsigned __int64)h % 64))) == 0)
			return false;

		int imax = ((int)events.size()) - 1;
		int imin = 0;
		while (imax >= imin)
		{
			int imid = (imax - imin) / 2 + imin;
			int cmp = CompEvents(events[imid], ev);
			if (cmp == 1)
				imin = imid + 1;
			else if (cmp == -1)
				imax = imid - 1;
			else {
				return true;
			}
		}
		return false;
	}

	/*void CountQualifies()
	{
		for (uint p = 0; p < TOP_NUM_QUALIFY; p++) {
			GetPlayerById(player_points[p].first DEBUGARGS).times_top++;
		}
	}*/

	void CountPoints(PointsCounter &pointscounter)
	{
		//return;
		bool got_players[MAX_PLAYER_ID];
		memset(got_players, 0, sizeof(got_players));
		for(uint i=0;i<player_points.size();i++) {
			auto &p=player_points[i];
			//pointscounter.AddPlayerCountsOccurance(p.first, p.second, (bool)(i<TOP_NUM_QUALIFY), 1);
			uint hits = 0;
			uint misses = 1;
			if (i < TOP_NUM_QUALIFY) {
				hits = 1;
				misses = 0;
			}
			pointscounter.AddPlayerCountsOccurance(p.first, p.second, hits, misses);
			assert(got_players[p.first]==false);
			got_players[p.first]=true;
		}
		for(uint p=0;p<MAX_PLAYER_ID;p++) {
			if(got_players[p]==true || players[p].player_id==0) continue;
			assert(players[p].wcs_points==0);
			pointscounter.AddPlayerCountsOccurance(p, 0, 0, 1);
		}
		pointscounter.AddCutoff(player_points[TOP_NUM_QUALIFY-1].second);
	}

	void SortEvents()
	{
		events.shrink_to_fit();
		sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
			//return a.Hash()>b.Hash();
			return CompEvents(a, b)>0;
		});
	}
	
	void CutPlayers()
	{
		/*if(player_points.size()>TOP_NUM_QUALIFY+4) {
			player_points.resize(TOP_NUM_QUALIFY+4);
		}*/
		if(player_points.size()>TOP_NUM_QUALIFY) {
			player_points.resize(TOP_NUM_QUALIFY);
		}
		player_points.shrink_to_fit();
		player_wcs_points.clear();
		player_wcs_points.shrink_to_fit();
	}

	void SortPlayers(RayRand64 &rng)
	{
		//_AddPlayerWCSPoints(players[300], 1);
		sort(player_wcs_points.begin(), player_wcs_points.end(), [](const pair<int, int> &a, const pair<int, int> &b)
		{
			if (a.second == b.second) {
				auto &playerA = GetPlayerById(a.first DEBUGARGS);
				auto &playerB = GetPlayerById(b.first DEBUGARGS);
				return playerA.ratings[R] > playerB.ratings[R];
			}
			return a.second > b.second;
		});
		//if (player_wcs_points.size() > TOP_NUM_QUALIFY * 2) player_wcs_points.resize(TOP_NUM_QUALIFY * 2);

		sort(player_points.begin(), player_points.end(), [this](const pair<int, int> &a, const pair<int, int> &b)
		{
			if (a.second == b.second) {
				uint wcs_points_a = 0;
				uint wcs_points_b = 0;
				if (a.second > 2500) {
					for (auto w : this->player_wcs_points) {
						if (w.first == a.first) wcs_points_a = w.second;
						if (w.first == b.first) wcs_points_b = w.second;
					}
				}
				if (wcs_points_a != wcs_points_b) return wcs_points_a > wcs_points_b;

				auto &playerA = GetPlayerById(a.first DEBUGARGS);
				auto &playerB = GetPlayerById(b.first DEBUGARGS);
				return playerA.ratings[R] > playerB.ratings[R];
			}
			return a.second > b.second;
		});
		if (player_points[TOP_NUM_QUALIFY - 1].second == player_points[TOP_NUM_QUALIFY].second) {
			auto a = player_points[TOP_NUM_QUALIFY - 1];//#16
			auto b = player_points[TOP_NUM_QUALIFY];//#17
			auto c = player_points[TOP_NUM_QUALIFY - 2];//#15
			auto d = player_points[TOP_NUM_QUALIFY + 1];//#18
			if (c.second == a.second) {//#15,#16,#17 tied, 2 players advance
				auto r = TripleTieBreaker(a.first, b.first, c.first, rng DEBUGARGS);
				player_points[TOP_NUM_QUALIFY - 2] = make_pair((unsigned short)r[0]->player_id,(short)a.second);
				player_points[TOP_NUM_QUALIFY - 1] = make_pair((unsigned short)r[1]->player_id, (short)a.second);
				player_points[TOP_NUM_QUALIFY] = make_pair((unsigned short)r[2]->player_id, (short)a.second);
			}
			else if (d.second == a.second) {//#16,#17,#18 tied, 1 player advances
				auto r = TripleTieBreaker(a.first, b.first, d.first, rng DEBUGARGS);
				player_points[TOP_NUM_QUALIFY - 1] = make_pair((unsigned short)r[0]->player_id, (short)a.second);
				player_points[TOP_NUM_QUALIFY] = make_pair((unsigned short)r[1]->player_id, (short)a.second);
				player_points[TOP_NUM_QUALIFY + 1] = make_pair((unsigned short)r[2]->player_id, (short)a.second);
			}
			else {//#16 and #17 tied, just a swap
				auto r = GlobalMatchPredict(GetPlayerById(a.first DEBUGARGS), GetPlayerById(b.first DEBUGARGS), 5);
				if (rng.dRand() >= r) {//b won, swap them
					player_points[TOP_NUM_QUALIFY - 1] = b;
					player_points[TOP_NUM_QUALIFY] = a;
				}
			}
		}

		/*if(rng.dRand()<0.5) {
			sort(player_points.begin(), player_points.end(), [this](const pair<int, int> &a, const pair<int, int> &b)
				 {
					 if (a.second == b.second) {
						 uint wcs_points_a = 0;
						 uint wcs_points_b = 0;
						 if (a.second > 2000) {
							 for (auto w : this->player_wcs_points) {
								 if (w.first == a.first) wcs_points_a = w.second;
								 if (w.first == b.first) wcs_points_b = w.second;
							 }
						 }
						 if (wcs_points_a != wcs_points_b) return wcs_points_a > wcs_points_b;

						 auto &playerA = GetPlayerById(a.first DEBUGARGS);
						 auto &playerB = GetPlayerById(b.first DEBUGARGS);
						 return playerA.ratings[R] > playerB.ratings[R];
					 }
					 return a.second > b.second;
				 });
		} else {
			sort(player_points.begin(), player_points.end(), [this](const pair<int, int> &a, const pair<int, int> &b)
			{
				if (a.second == b.second) {
					uint wcs_points_a = 0;
					uint wcs_points_b = 0;
					if (a.second > 2000) {
						for (auto w : this->player_wcs_points) {
							if (w.first == a.first) wcs_points_a = w.second;
							if (w.first == b.first) wcs_points_b = w.second;
						}
					}
					if (wcs_points_a != wcs_points_b) return wcs_points_a > wcs_points_b;

					auto &playerA = GetPlayerById(a.first DEBUGARGS);
					auto &playerB = GetPlayerById(b.first DEBUGARGS);
					return playerA.ratings[R] < playerB.ratings[R];
				}
				return a.second > b.second;
			});
		}*/
		//sort the qualified players again with using points from WCS proper for tiebreakers...
	}

	const bool IsQualified(uint p_id)
	{
		for (size_t i = 0; i < TOP_NUM_QUALIFY; i++) {
			if (player_points[i].first == p_id)
				return true;
		}
		return false;
	}

	string Output();
};
