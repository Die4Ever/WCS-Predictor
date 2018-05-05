
class MatchBase
{
public:
	byte completion;
	vector<byte> old_results;
	//uint timestamp;
	uint upcoming;
	//string name;
	//bool completed;

	MatchBase()
	{
		completion=0;
		//timestamp=0;
		upcoming=0;
		//completed=false;
	}

	double MatchPredict(Player &A, Player &B, int best_of, uint seq)
	{
		if(old_results.size()>seq) {
			byte r = old_results[seq];
			if(r==1)
				return 1.1;
			if(r==2)
				return -0.1;
		}
		return GlobalMatchPredict(A, B, best_of);
	}

	double MatchPredictNoOldResults(Player &A, Player &B, int best_of)
	{
		return GlobalMatchPredict(A, B, best_of);
	}
};

class SwissGroup : public MatchBase
{
public:
	Player* players[4];

	SwissGroup()
	{
		//players[0]=players[1]=players[2]=players[3]=NULL;
	}

	void ReadOldResults(ScrapedGroup &info, vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, uint roundof, bool skipwalkovers=true)
	{
		//timestamp=info.timestamp;
		//name=info.name;

		int total=1;
		int walkovers=0;
		for(uint i=0;i<4;i++) {
			if(players[i] && players[i]->player_id==0) {
				walkovers++;
			}
		}
		if(walkovers==1) {
			total=3;
		}
		for(int i=0;i<5;i++) {
			if(info.match_winners[i]==0 || info.match_losers[i]==0)
				total--;
		}
		if (total <= 0) {
			//if(timestamp>(uint)time(0)-86400*30) {
			UpcomingMatch upmatch;
			upcoming = upmatch.id;
			upmatch.timestamp = info.timestamp;
			upmatch.t_id = info.t_id;
			upmatch.round = roundof;
			string against;
			for (int p = 0; p < 4; p++) {
				if (players[p] == NULL) {
					upmatch.players[p] = 0;
					continue;
				}
				upmatch.players[p] = players[p]->player_id;
				against+=players[p]->name.ToString()+", ";
			}
			upcoming_matches.push_back(upmatch);
			cerr << "upcoming match with "<<against<<" in "<<info.t_id<<"\n";
			//}
			if(total<=-3) return;
			if (AFK) { cerr << "skipping ReadOldResults on ongoing match "<<info.name<<" because AFK mode is enabled, total=="<<total<<"\n"; return; }
			//return;
		}
		uint player_map[4];//player_map[info_player_id]
		uint rplayer_map[4];//rplayer_map[p]==info_player_id
		for(uint p=0;p<4;p++) {
			for(uint i=0;i<4;i++) {
				if(players[p]->MatchName(info.players[i])) {
					player_map[i]=p;//does this need to be reversed? lol
					rplayer_map[p]=i;//sure why not
				}
			}
		}

		uint A,B,C,D, wA,wB, lA,lB, fA,fB;
		A=rplayer_map[0]+1;
		B=rplayer_map[1]+1;
		C=rplayer_map[2]+1;
		D=rplayer_map[3]+1;

		int match=0;
		
		if(C==info.match_winners[0] || D==info.match_winners[0]) {
			auto tw=info.match_winners[0];
			auto tl=info.match_losers[0];
			auto tws=info.winner_scores[0];
			auto tls=info.loser_scores[0];
			info.match_winners[0]=info.match_winners[1];
			info.match_losers[0]=info.match_losers[1];
			info.winner_scores[0]=info.winner_scores[1];
			info.loser_scores[0]=info.loser_scores[1];
			info.match_winners[1]=tw;
			info.match_losers[1]=tl;
			info.winner_scores[1]=tws;
			info.loser_scores[1]=tls;
		}
		
		if(players[player_map[A-1]]->player_id==0) {
			old_results.push_back(2);
			if(skipwalkovers) match--;
			wA=B;
			lA=A;
		} else if(players[player_map[B-1]]->player_id==0) {
			old_results.push_back(1);
			if(skipwalkovers) match--;
			wA=A;
			lA=B;
		} else if(A==info.match_winners[match]) {
			old_results.push_back(1);
			wA=A;
			lA=B;
			PreviousMatch prevmatch(players[player_map[wA-1]], players[player_map[lA-1]], match, info, roundof);
			prev_matches.push_back(prevmatch);
		} else if(B==info.match_winners[0]) {
			old_results.push_back(2);
			wA=B;
			lA=A;
			PreviousMatch prevmatch(players[player_map[wA-1]], players[player_map[lA-1]], match, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			assert(0);
			old_results.push_back(0);
			//completed=false;
		}

		match++;
		if(players[player_map[C-1]]->player_id==0) {
			old_results.push_back(2);
			if(skipwalkovers) match--;
			wB=D;
			lB=C;
		} else if(players[player_map[D-1]]->player_id==0) {
			old_results.push_back(1);
			if(skipwalkovers) match--;
			wB=C;
			lB=D;
		} else if(C==info.match_winners[match]) {
			old_results.push_back(1);
			wB=C;
			lB=D;
			PreviousMatch prevmatch(players[player_map[wB-1]], players[player_map[lB-1]], match, info, roundof);
			prev_matches.push_back(prevmatch);
		}
		else if(D==info.match_winners[match]) {
			old_results.push_back(2);
			wB=D;
			lB=C;
			PreviousMatch prevmatch(players[player_map[wB-1]], players[player_map[lB-1]], match, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			//cerr << players[player_map[info.match_winners[match]]]->name<<" beat "<<players[player_map[info.match_losers[match]]]->name<<"\n";
			assert(0);
			old_results.push_back(0);
			//completed=false;
		}

		match++;
		auto wmatch=match;
		auto lmatch=match+1;
		auto fmatch=match+2;

		if(
			(info.match_winners[match]!=wA && info.match_winners[match]!=wB)
		&&
			(info.match_winners[match]==lA || info.match_winners[match]==lB) ) {
			wmatch=match+1;
			lmatch=match;
		}
		if(wA==info.match_winners[wmatch]) {
			old_results.push_back(1);
			fA=wB;
			PreviousMatch prevmatch(players[player_map[wA-1]], players[player_map[wB-1]], wmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		}
		else if(wB==info.match_winners[wmatch]) {
			old_results.push_back(2);
			fA=wA;
			PreviousMatch prevmatch(players[player_map[wB-1]], players[player_map[wA-1]], wmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			//assert(0);
			old_results.push_back(0);
			//completed=false;
		}

		match++;
		if(players[player_map[lA-1]]->player_id==0) {
			old_results.push_back(2);
			if(skipwalkovers) {
				match--;
				fmatch--;
			}
			fB=lB;
		} else if(players[player_map[lB-1]]->player_id==0) {
			old_results.push_back(1);
			if(skipwalkovers) {
				match--;
				fmatch--;
			}
			fB=lA;
		} else if(lA==info.match_winners[lmatch]) {
			old_results.push_back(1);
			fB=lA;
			PreviousMatch prevmatch(players[player_map[lA-1]], players[player_map[lB-1]], lmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		}
		else if(lB==info.match_winners[lmatch]) {
			old_results.push_back(2);
			fB=lB;
			PreviousMatch prevmatch(players[player_map[lB-1]], players[player_map[lA-1]], lmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			//assert(0);
			old_results.push_back(0);
			//completed=false;
		}

		match++;
		if(fA==info.match_winners[fmatch]) {
			old_results.push_back(1);
			PreviousMatch prevmatch(players[player_map[fA-1]], players[player_map[fB-1]], fmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		}
		else if(fB==info.match_winners[fmatch]) {
			old_results.push_back(2);
			PreviousMatch prevmatch(players[player_map[fB-1]], players[player_map[fA-1]], fmatch, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			//assert(0);
			old_results.push_back(0);
			//completed=false;
		}
		if(old_results.size()) completion=255;
		for(auto o : old_results) if(o==0) completion=1;
	}

	array<Player*,4> predict(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,4> placings;
		Player *wA,*wB,*lA,*lB,*fA,*fB;

		double r;
		r = MatchPredict(*players[0], *players[1], best_of, 0);
		if(rng.dRand()<r) {
			wA=players[0];
			lA=players[1];
		} else {
			wA=players[1];
			lA=players[0];
		}

		r = MatchPredict(*players[2], *players[3], best_of, 1);
		if(rng.dRand()<r) {
			wB=players[2];
			lB=players[3];
		} else {
			wB=players[3];
			lB=players[2];
		}

		//winners' match
		r = MatchPredict(*wA, *wB, best_of, 2);
		if(rng.dRand()<r) {
			fA=wB;
			placings[0]=wA;
		} else {
			fA=wA;
			placings[0]=wB;
		}

		//losers' match
		r = MatchPredict(*lA, *lB, best_of, 3);
		if(rng.dRand()<r) {
			fB=lA;
			placings[3]=lB;
		} else {
			fB=lB;
			placings[3]=lA;
		}

		//final match
		r = MatchPredict(*fA, *fB, best_of, 4);
		if(rng.dRand()<r) {
			placings[1]=fA;
			placings[2]=fB;
		} else {
			placings[1]=fB;
			placings[2]=fA;
		}

		return placings;
	}

	array<Player*,4> PredictNoOldResults(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,4> placings;
		Player *wA,*wB,*lA,*lB,*fA,*fB;

		double r;
		r = MatchPredictNoOldResults(*players[0], *players[1], best_of);
		if(rng.dRand()<r) {
			wA=players[0];
			lA=players[1];
		} else {
			wA=players[1];
			lA=players[0];
		}

		r = MatchPredictNoOldResults(*players[2], *players[3], best_of);
		if(rng.dRand()<r) {
			wB=players[2];
			lB=players[3];
		} else {
			wB=players[3];
			lB=players[2];
		}

		//winners' match
		r = MatchPredictNoOldResults(*wA, *wB, best_of);
		if(rng.dRand()<r) {
			fA=wB;
			placings[0]=wA;
		} else {
			fA=wA;
			placings[0]=wB;
		}

		//losers' match
		r = MatchPredictNoOldResults(*lA, *lB, best_of);
		if(rng.dRand()<r) {
			fB=lA;
			placings[3]=lB;
		} else {
			fB=lB;
			placings[3]=lA;
		}

		//final match
		r = MatchPredictNoOldResults(*fA, *fB, best_of);
		if(rng.dRand()<r) {
			placings[1]=fA;
			placings[2]=fB;
		} else {
			placings[1]=fB;
			placings[2]=fA;
		}

		return placings;
	}

	uint GetNumPlayers()
	{
		return 4u;
	}

	uint GetNumPlayersAdvance()
	{
		return 2u;
	}
};

template<uint num_players, uint num_players_advance>
class RoundRobin : public MatchBase
{
public:
	Player *players[num_players];
	array<pair<Player*,int>, num_players > match_scores;

	RoundRobin()
	{
		for(uint i=0;i<num_players;i++)
			players[i]=NULL;
		for(uint i=0;i<num_players;i++) {
			match_scores[i].first=players[i];
			match_scores[i].second=0;
		}
	}
	//will need to create an array of ints on the stack to track everyone's match scores and map scores
		//need to look at head to head for tiebreakers, need to program in handling of 3 way ties
			//if I detect a tie breaker, can I just create a temporary round robin group of 3 recursively?
	uint GetNumPlayers()
	{
		return num_players;
	}

	uint GetNumPlayersAdvance()
	{
		return num_players_advance;
	}

	void ReadOldResults(ScrapedGroup &info, vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, uint roundof)
	{
		//timestamp=info.timestamp;
		//name=info.name;
		bool completed=false;

		uint player_map[num_players+1];//player_map[info_player_id]
		//uint rplayer_map[num_players];//rplayer_map[p]==info_player_id
		for(uint p=0;p<num_players;p++) {
			for(uint i=0;i<num_players;i++) {
				if(players[p]->MatchName(info.players[i])) {
					player_map[i+1]=p;//does this need to be reversed? lol
					//rplayer_map[p]=i;//sure why not
				}
			}
		}

		uint num_matches=0;
		for(uint a=0;a<num_players;a++) {
			for(uint b=a+1;b<num_players;b++) {
				if( (players[a]==NULL || players[a]->player_id>0) && (players[b]==NULL || players[b]->player_id>0) ) {
					num_matches++;
				}
			}
		}

		uint last_match=0;
		completed=false;
		for(uint m=num_matches-2;m<scraped_group_matches;m++) {
					if( info.match_winners[m]!=0 ) {
						completed=true;
					}
		}
		if (completed == false) {
			UpcomingMatch upmatch;
			upcoming = upmatch.id;
			upmatch.timestamp = info.timestamp;
			upmatch.t_id = info.t_id;
			upmatch.round = roundof;
			string against;
			for (int p = 0; p < num_players; p++) {
				if (players[p] == NULL) {
					upmatch.players[p] = 0;
					continue;
				}
				upmatch.players[p] = players[p]->player_id;
				against+=players[p]->name.ToString()+", ";
			}
			upcoming_matches.push_back(upmatch);
			cerr << "upcoming match with "<<against<<" in "<<info.t_id<<"\n";
			//}
			//return;
		}

		for(uint a=0;a<num_players;a++) {
			for(uint b=a+1;b<num_players;b++) {
				bool found=false;
				for(uint m=0;m<scraped_group_matches;m++) {
					if( info.match_winners[m]==0 ) continue;
					if( player_map[info.match_winners[m]] != a && player_map[info.match_losers[m]] != a ) continue;
					if( player_map[info.match_winners[m]] != b && player_map[info.match_losers[m]] != b ) continue;

					found=true;
					last_match=max(last_match,m);
					if( player_map[info.match_winners[m]] == a) {
						old_results.push_back(1);
						PreviousMatch prevmatch(players[a], players[b], m, info, roundof);
					} else if( player_map[info.match_winners[m]] == b) {
						old_results.push_back(2);
						PreviousMatch prevmatch(players[b], players[a], m, info, roundof);
					} else {
						old_results.push_back(0);
						completed=false;
					}

					break;
				}
				if(found==false) {
					old_results.push_back(0);
					completed=false;
				}
			}
		}
		for(uint m=last_match+1;m<scraped_group_matches;m++) {
			if( info.match_winners[m] != 0 && info.match_losers[m] != 0 ) DebugBreak();
		}
		if(old_results.size()) completion=1;
		if(old_results.size() && completed) completion=255;
		for(auto o : old_results) if(o==0) completion=1;
	}

	array<Player*,num_players> predict(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,num_players> placings;
		auto tmatch_scores=match_scores;
		for(uint i=0;i<num_players;i++) {
			tmatch_scores[i].first=players[i];
		}

		uint match_id=0;
		for (uint a = 0; a < num_players; a++) {
			for (uint b = a + 1; b < num_players; b++) {
				double r;
				r = MatchPredict(*players[a], *players[b], best_of, match_id++);
				if (rng.dRand() < r) {
					tmatch_scores[a].second++;
					tmatch_scores[b].second--;
				} else {
					tmatch_scores[b].second++;
					tmatch_scores[a].second--;
				}
			}
		}

		stable_sort(tmatch_scores.begin(), tmatch_scores.end(), [](const pair<Player*,int> a, const pair<Player*,int> b) {
			if(a.second==b.second) return a.first->ratings[R]>b.first->ratings[R];
			return a.second>b.second;
		});
		for(uint i=0;i<num_players;i++) {
			placings[i]=tmatch_scores[i].first;
			//cerr << placings[i]->name << " got "<<i+1<<" place with "<<match_scores[i].second<<" points\n";
		}
		return placings;
	}

	array<Player*,num_players> PredictNoOldResults(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,num_players> placings;
		auto tmatch_scores=match_scores;
		for(uint i=0;i<num_players;i++) {
			tmatch_scores[i].first=players[i];
		}

		uint match_id=0;
		for (uint a = 0; a < num_players; a++) {
			for (uint b = a + 1; b < num_players; b++) {
				double r;
				r = MatchPredictNoOldResults(*players[a], *players[b], best_of);
				if (rng.dRand() < r) {
					tmatch_scores[a].second++;
					tmatch_scores[b].second--;
				} else {
					tmatch_scores[b].second++;
					tmatch_scores[a].second--;
				}
			}
		}

		stable_sort(tmatch_scores.begin(), tmatch_scores.end(), [](const pair<Player*,int> a, const pair<Player*,int> b) {
			if(a.second==b.second) return a.first->ratings[R]>b.first->ratings[R];
			return a.second>b.second;
		});
		for(uint i=0;i<num_players;i++) {
			placings[i]=tmatch_scores[i].first;
			//cerr << placings[i]->name << " got "<<i+1<<" place with "<<match_scores[i].second<<" points\n";
		}
		return placings;
	}
};

class SingleMatch : public MatchBase
{
public:
	Player *players[2];

	SingleMatch()
	{
		players[0]=players[1]=NULL;
	}

	void EasyAddUpcoming(vector<UpcomingMatch> &upcoming_matches, uint iTimestamp, uint t_id, uint roundof, bool estimated_time=false)
	{
		//timestamp = iTimestamp;
		UpcomingMatch upmatch;
		upcoming = upmatch.id;
		upmatch.timestamp = iTimestamp;
		upmatch.t_id = t_id;
		upmatch.round = roundof;
		upmatch.estimated_time=estimated_time;
		string against;
		for (int p = 0; p < 2; p++) {
			if (players[p] == NULL) {
				upmatch.players[p] = 0;
				continue;
			}
			upmatch.players[p] = players[p]->player_id;
			against+=players[p]->name.ToString()+", ";
		}
		upcoming_matches.push_back(upmatch);
		cerr << "upcoming match with "<<against<<" in "<<t_id<<"\n";
	}

	void ReadOldResults(ScrapedGroup &info, vector<PreviousMatch> &prev_matches, vector<UpcomingMatch> &upcoming_matches, uint roundof)
	{
		//timestamp=info.timestamp;
		//name=info.name;
		uint player_map[2];//player_map[info_player_id]
		uint rplayer_map[2];//rplayer_map[p]==info_player_id
		for(uint p=0;p<2;p++) {
			for(uint i=0;i<2;i++) {
				if(players[p]->MatchName(info.players[i])) {
					player_map[i]=p;//does this need to be reversed? lol
					rplayer_map[p]=i;//sure why not
				}
			}
		}

		int A,B;
		A=rplayer_map[0]+1;
		B=rplayer_map[1]+1;

		int total=0;
		for(int i=0;i<2;i++) {
			total+=info.match_winners[i];
			total+=info.match_losers[i];
		}
		if (total == 0) {
			//if(timestamp>(uint)time(0)-86400*7) {
			UpcomingMatch upmatch;
			upcoming = upmatch.id;
			upmatch.timestamp = info.timestamp;
			upmatch.t_id = info.t_id;
			upmatch.round = roundof;
			string against;
			for (int p = 0; p < 2; p++) {
				if (players[p] == NULL) {
					upmatch.players[p] = 0;
					continue;
				}
				upmatch.players[p] = players[p]->player_id;
				against+=players[p]->name.ToString()+", ";
			}
			upcoming_matches.push_back(upmatch);
			cerr << "upcoming match with "<<against<<" in "<<info.t_id<<"\n";
			//}
			return;
		}

		//completed=true;
		if(A==info.match_winners[0]) {
			old_results.push_back(1);
			PreviousMatch prevmatch(players[player_map[A-1]], players[player_map[B-1]], 0, info, roundof);
			prev_matches.push_back(prevmatch);
		} else if(B==info.match_winners[0]) {
			old_results.push_back(2);
			PreviousMatch prevmatch(players[player_map[B-1]], players[player_map[A-1]], 0, info, roundof);
			prev_matches.push_back(prevmatch);
		} else {
			old_results.push_back(0);
			//completed=false;
		}
		if(old_results.size()) completion=255;
		for(auto o : old_results) if(o==0) completion=1;
	}

	array<Player*,2> predict(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,2> placings;

		double r;
		r = MatchPredict(*players[0], *players[1], best_of, 0);
		if(rng.dRand()<r) {
			placings[0]=players[0];
			placings[1]=players[1];
		} else {
			placings[0]=players[1];
			placings[1]=players[0];
		}

		return placings;
	}

	array<Player*,2> PredictNoOldResults(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,2> placings;

		double r;
		r = MatchPredictNoOldResults(*players[0], *players[1], best_of);
		if(rng.dRand()<r) {
			placings[0]=players[0];
			placings[1]=players[1];
		} else {
			placings[0]=players[1];
			placings[1]=players[0];
		}

		return placings;
	}

	uint GetNumPlayers()
	{
		return 2u;
	}

	uint GetNumPlayersAdvance()
	{
		return 1u;
	}
};

class DEFinals : public SingleMatch
{
public:
	array<Player*,2> predict(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,2> placings;

		double r;
		r = MatchPredict(*players[0], *players[1], best_of, 0);
		if(rng.dRand()<r) {
			placings[0]=players[0];
			placings[1]=players[1];
		} else {
			return SingleMatch::predict(best_of, rng DEBUGARGS);
			//placings[0]=players[1];
			//placings[1]=players[0];
		}

		return placings;
	}

	array<Player*,2> PredictNoOldResults(int best_of, RayRand64 &rng DEBUGARGSDEF)
	{
		array<Player*,2> placings;

		double r;
		r = MatchPredictNoOldResults(*players[0], *players[1], best_of);
		if(rng.dRand()<r) {
			placings[0]=players[0];
			placings[1]=players[1];
		} else {
			return SingleMatch::PredictNoOldResults(best_of, rng DEBUGARGS);
			//placings[0]=players[1];
			//placings[1]=players[0];
		}

		return placings;
	}
};

class RoundBase
{
public:
	int points_for_placing[6];//usually will only use 2, 4, maybe 5 of these
	uint match_placing_to_tournament_placing[6];
	int best_of;
	byte completion;

	RoundBase()
	{
		completion=1;//0;//1 for safety...
		best_of=3;
		for(int i=0;i<6;i++) {
			points_for_placing[i]=0;
			match_placing_to_tournament_placing[i]=0;
		}
	}
};

template<uint round_of, class MatchType, uint num_matches, class NextAdvancement, bool WCS=false>
class Round : public RoundBase
{
public:
	MatchType matches[num_matches];

	uint RoundOf()
	{
		return round_of;
	}

	void ResetPlacings()
	{
		uint num_adv = matches[0].GetNumPlayersAdvance();
		for (uint i = num_adv; i<6; i++) {
			match_placing_to_tournament_placing[i] = round_of;
		}
	}

	Round()
	{
		STATIC_ASSERT(round_of>num_matches);
		ResetPlacings();
	}

	void InitCompletion()
	{
		completion=0;
		for(auto &m : matches) {
			if(m.completion) completion++;
		}
		uint fullycompleted=0;
		for(auto &m : matches) {
			if(m.completion==255) fullycompleted++;
		}
		if(fullycompleted==num_matches) completion=255;
	}

	MatchType *GetMatch(array<string,8> &names)
	{
		for(uint m=0;m<num_matches;m++) {
			uint found=0;
			uint players = matches[m].GetNumPlayers();
			for (uint p = 0; p < players; p++) {
				//string player_name = ToLower(matches[m].players[p]->name.ToString());
				if(matches[m].players[p]) {
					for (uint i = 0; i < players; i++) {
						//string tname = ToLower(names[i]);
						if (matches[m].players[p]->MatchName(names[i])) found++;
					}
				} else {
					for (uint i=0;i<players;i++) {
						string tname = ToLower(names[i]);
						if(tname=="tbd"/* || tname==""*/) {
							found++;
							break;
						}
					}
				}
			}
			if(found==players) {
				assert(matches[m].upcoming==0);
				assert(matches[m].completion!=255);//make sure we didn't read this one already?
				assert(matches[m].old_results.size()==0);
				return &matches[m];
			}
		}
		return NULL;
	}

	void ClearPlacings()
	{
		uint num_adv=matches[0].GetNumPlayersAdvance();
		for(uint i=0/*num_adv*/;i<6;i++) {//why did it skip the winners? it should only make a difference for finals, why would I want that?
			match_placing_to_tournament_placing[i]=0;
		}
	}

	void AcceptAdvancements(array<Player*, round_of> &players DEBUGARGSDEF)
	{
		for(uint i=0;i<num_matches;i++) {
			for(uint p=0;p<matches[i].GetNumPlayers();p++) {
				uint from_slot=i*(round_of/num_matches)+p;
				assert( from_slot < players.size() );
				matches[i].players[p]=players[from_slot];
			}
		}
	}

	void PredictNoOldResults(Simulation &sim, uint t_id, NextAdvancement &adv, RayRand64 &rng DEBUGARGSDEF)
	{
		for(uint i=0;i<num_matches;i++) {
			uint num_adv=matches[i].GetNumPlayersAdvance();
			uint num_players=matches[i].GetNumPlayers();
			auto results = matches[i].PredictNoOldResults(best_of, rng DEBUGARGS);
			//bool always = false;//matches[i].completed=255;
			for(uint p=0;p<results.size();p++) {
				adv.AddPlayer(results[p], p, i, p, num_players, num_adv, rng );

				uint tplacing=match_placing_to_tournament_placing[p];
				if(matches[i].upcoming /*&& always==false*/) {
					if(p<num_adv)
						sim.AddEvent(matches[i].upcoming, results[p]->player_id, UPCOMING_WIN);
					else
						sim.AddEvent(matches[i].upcoming, results[p]->player_id, UPCOMING_LOSE);
					if(tplacing==1) {
						sim.AddEvent(t_id, results[p]->player_id, tplacing);
					}
				} else if(tplacing /*&& always==false*/) {
					sim.AddEvent(t_id, results[p]->player_id, tplacing);
					if(BDEBUG && t_id==0) {
						cerr << " - "<<results[p]->name << " got "<<tplacing<<", match "<<i<<"\n";
					}
				}
				int points=points_for_placing[p];
				if(points) {
					if (WCS) sim.AddPlayerWCSPoints(*results[p], points);
					else sim.AddPlayerPoints(*results[p], points);
				}
			}
		}
	}

	void predict(Simulation &sim, uint t_id, NextAdvancement &adv, RayRand64 &rng DEBUGARGSDEF)
	{
		if(completion==0) return PredictNoOldResults(sim, t_id, adv, rng DEBUGARGS);//this doesn't help?

		for(uint i=0;i<num_matches;i++) {
			uint num_adv=matches[i].GetNumPlayersAdvance();
			uint num_players=matches[i].GetNumPlayers();
			auto results = matches[i].predict(best_of, rng DEBUGARGS);
			bool always = matches[i].completion==255;
			for(uint p=0;p<results.size();p++) {
				adv.AddPlayer(results[p], p, i, p, num_players, num_adv, rng );

				uint tplacing=match_placing_to_tournament_placing[p];
				if(matches[i].upcoming && always==false) {
					if(p<num_adv)
						sim.AddEvent(matches[i].upcoming, results[p]->player_id, UPCOMING_WIN);
					else
						sim.AddEvent(matches[i].upcoming, results[p]->player_id, UPCOMING_LOSE);
					if(tplacing==1) {
						sim.AddEvent(t_id, results[p]->player_id, tplacing);
					}
				} else if(tplacing && always==false) {
					sim.AddEvent(t_id, results[p]->player_id, tplacing);
					if(BDEBUG && t_id==0) {
						cerr << " - "<<results[p]->name << " got "<<tplacing<<", match "<<i<<"\n";
					}
				}
				int points=points_for_placing[p];
				if(points) {
					if (WCS) sim.AddPlayerWCSPoints(*results[p], points);
					else sim.AddPlayerPoints(*results[p], points);
					/*if (BDEBUG && always) {
						DEBUGOUT(FATAL, points, results[p]->name.ToString() << " in " << TournamentNames[t_id]);
					}*/
				}
			}
		}
	}
};

class NullAdvancing
{
public:
	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
	}
};

template<uint advancing, uint falling>
class RandomAdvancement
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;

	RandomAdvancement()
	{
		clear();
	}

	void clear()
	{
		for(uint i=0;i<advancing;i++)
			advancing_players[i]=NULL;
		for(uint i=0;i<falling;i++)
			falling_players[i]=NULL;
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=0;//mslot*top_advances+pslot;
		uint tries=0;
		do {
			aslot=rng.iRand64HQ()%advancing;
			assert(aslot<advancing);
			if(BDEBUG) {
				tries++;
				if(tries>10000) throw rayexception("AddAdvancingPlayer over 10000 tries!" DEBUGARGS);
			}
		} while(advancing_players[aslot]);
		advancing_players[aslot]=player;
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint fslot=0;//mslot*(num_players-top_advances)+(pslot-top_advances);
		uint tries=0;
		do {
			fslot=rng.iRand64HQ()%falling;
			assert(fslot<falling);
			if(BDEBUG) {
				tries++;
				if(tries>10000) throw rayexception("AddAdvancingPlayer over 10000 tries!" DEBUGARGS);
			}
		} while(falling_players[fslot]);
		falling_players[fslot]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint advancing, uint falling>
class Outside1stsInside2nds
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;
	uint last_falling_slot;

	Outside1stsInside2nds()
	{
		last_falling_slot=0;
		for(uint i=0;i<advancing;i++)
			advancing_players[i]=NULL;
		for(uint i=0;i<falling;i++)
			falling_players[i]=NULL;
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=0;
		uint num_groups=(advancing+falling)/num_players;

		if(placing==0) {
			if(mslot< num_groups/2 ) {
				aslot=mslot*num_players;
			} else {
				aslot=(num_groups-mslot-1)*num_players +2;
			}
		} else {
			if(mslot< num_groups/2) {
				aslot=(num_groups/2-mslot-1)*num_players +3;
			} else {
				aslot=(mslot-num_groups/2)*num_players +1;
			}
		}
		advancing_players[aslot]=player;
		//cerr << "Group "<<(mslot+1)<<"'s "<<(placing+1)<<" place advanced to group "<<(aslot/4+1)<<" slot "<<((aslot%4)+1)<<"\n";
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		/*uint fslot=0;//mslot*(num_players-top_advances)+(pslot-top_advances);
		uint tries=0;
		do {
			fslot=rng.iRand64HQ()%falling;
			assert(fslot<falling);
			if(BDEBUG) {
				tries++;
				if(tries>1000) throw rayexception("AddAdvancingPlayer over 1000 tries!" DEBUGARGS);
			}
		} while(falling_players[fslot]);*/
		uint fslot=last_falling_slot++;
		falling_players[fslot]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint advancing, uint falling>
class A1vsB2
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;
	uint last_falling_slot;

	A1vsB2()
	{
		last_falling_slot=0;
		for(uint i=0;i<advancing;i++)
			advancing_players[i]=NULL;
		for(uint i=0;i<falling;i++)
			falling_players[i]=NULL;
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=0;//mslot*top_advances+pslot;
		if(placing==(mslot%2)) {
			aslot=mslot;
		} else {
			aslot=(mslot+(advancing/top_advances));
		}
		advancing_players[aslot]=player;
		//cerr << "Group "<<("ABCD"[mslot])<<"'s "<<(placing+1)<<" place advanced to bracket "<<(aslot/2+1)<<" slot "<<((aslot%2)+1)<<"\n";
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		/*uint fslot=0;//mslot*(num_players-top_advances)+(pslot-top_advances);
		uint tries=0;
		do {
			fslot=rng.iRand64HQ()%falling;
			assert(fslot<falling);
			if(BDEBUG) {
				tries++;
				if(tries>1000) throw rayexception("AddAdvancingPlayer over 1000 tries!" DEBUGARGS);
			}
		} while(falling_players[fslot]);*/
		uint fslot=last_falling_slot++;
		falling_players[fslot]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint advancing, uint falling>
class StraightAdvancement
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;

	StraightAdvancement()
	{
		if(BDEBUG) {
			for(uint i=0;i<advancing;i++)
				advancing_players[i]=NULL;
			for(uint i=0;i<falling;i++)
				falling_players[i]=NULL;
		}
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=mslot;
		assert(aslot<advancing);
		advancing_players[aslot]=player;
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint fslot=mslot;
		assert(fslot<falling);
		falling_players[fslot]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint advancing, uint falling>
class StraightGroupAdvancement
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;
	uint last_falling_slot;

	StraightGroupAdvancement()
	{
		last_falling_slot=0;
		if(BDEBUG) {
			for(uint i=0;i<advancing;i++)
				advancing_players[i]=NULL;
			for(uint i=0;i<falling;i++)
				falling_players[i]=NULL;
		}
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=mslot*2;
		if(pslot==1) {
			uint num_matches=advancing/top_advances;
			aslot=(num_matches-mslot-1)*2+1;
		}//[a1,d2,b1,c2, c1,b2,d1,a2]
		assert(aslot<advancing);
		advancing_players[aslot]=player;
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		falling_players[last_falling_slot++]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint advancing, uint falling>
class MixedAdvancement
{
public:
	array<Player*, advancing> advancing_players;
	array<Player*, falling> falling_players;
	uint last_falling_slot;
	uint last_1st_advancing_slot;
	uint last_2nd_advancing_slot;

	MixedAdvancement()
	{
		last_falling_slot=0;
		last_1st_advancing_slot=0;
		last_2nd_advancing_slot=advancing-1;
		/*if(BDEBUG)
		{
			for(uint i=0;i<advancing;i++)
				advancing_players[i]=NULL;
			for(uint i=0;i<falling;i++)
				falling_players[i]=NULL;
		}*/
	}

	void clear()
	{
		for(uint i=0;i<advancing;i++)
			advancing_players[i]=NULL;
		for(uint i=0;i<falling;i++)
			falling_players[i]=NULL;
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=0;
		if(pslot==0) {
			aslot=last_1st_advancing_slot++;
		} else {
			aslot=last_2nd_advancing_slot--;
		}
		assert(aslot<advancing);
		//assert(advancing_players[aslot]==NULL);
		advancing_players[aslot]=player;
	}

	void AddFallingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		falling_players[last_falling_slot++]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		if(placing<top_advances) {
			AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		} else {
			AddFallingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
		}
	}
};

template<uint players>
class SeedingAdvancement
{
public:
	array<Player*, players> seeded_players;

	SeedingAdvancement()
	{
		if(BDEBUG) {
			for(uint i=0;i<players;i++)
				seeded_players[i]=NULL;
		}
	}

	void AddAdvancingPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		uint aslot=mslot*num_players +pslot;
		assert(aslot<players);
		seeded_players[aslot]=player;
	}

	void AddPlayer(Player *player, uint placing, uint mslot, uint pslot, uint num_players, uint top_advances, RayRand64 &rng)
	{
		AddAdvancingPlayer(player, placing, mslot, pslot, num_players, top_advances, rng);
	}
};

array<string, 1024> TournamentNames;
class TournamentBase
{
public:
	uint t_id;
	byte completion;
	//TournamentName name;
	const char *name;

	TournamentBase(const char *cname, uint TID)
	{
		name=cname;
		completion=0;
		t_id=TID;
		if (t_id>0 && t_id<TournamentNames.size()) TournamentNames[t_id] = cname;
	}

	void init(DEBUGARGDEF)
	{
	}

	void predict(Simulation &sim DEBUGARGSDEF)
	{
		DEBUGARG;
	}
};


void ScrapeLiquipediaBracketsSimple(string &res, int t_id, vector<ScrapedGroup> &smatches, const char* starter = "-->")
{
	//DEBUGOUTMARKER(WARN);
	vector<string> textarea = SuperSplit(res.c_str(), "<textarea%>%</textarea>");
	//cerr << "\n\n"<< textarea[1] << "\n\n";
	vector<string> split = SuperSplit_R(strstr(textarea[1].c_str(), starter) + strlen(starter), "%|R%\n%");

	for (uint i = 1; i + 1<split.size(); i += 4) {
		if (strstr(split[i].c_str(), "details={{BracketMatchSummary")) {
			vector<string> vars = SuperSplit_R(split[i + 1].c_str(), "|%=%");
			for (uint a = 0; a + 1<vars.size(); a++) {
				if (vars[a] == "date" && vars[a + 1].length()>1 && smatches.back().timestamp == 0) {
					//DEBUGOUTMARKER(WARN);
					smatches.back().timestamp = Timestamp(vars[a + 1]);
					//DEBUGOUTMARKER(WARN);
					break;
				}
			}
			i -= 2;
			//DEBUGOUTMARKER(WARN);
			continue;
		}
		if (i + 2 >= split.size()) continue;
		ScrapedGroup sgroup;
		sgroup.t_id = t_id;
		split[i] = "|R" + split[i];
		split[i + 2] = "|R" + split[i + 2];
		vector<string> vars1 = SuperSplit_R(split[i].c_str(), "|%=%");
		vector<string> vars2 = SuperSplit_R(split[i + 2].c_str(), "|%=%");
		//DEBUGOUTMARKER(WARN);

		string player = vars1[2];
		for (size_t s = player.length() - 1; s < 1000; s--) {
			if (player[s] != ' ') {
				player = player.substr(0, s + 1);
				break;
			}
			else {
				player = player.substr(0, s);
			}
		}
		sgroup.players[0] = player;
		player = vars2[2];
		for (size_t s = player.length() - 1; s < 1000; s--) {
			if (player[s] != ' ') {
				player = player.substr(0, s + 1);
				break;
			}
			else {
				player = player.substr(0, s);
			}
		}
		sgroup.players[1] = player;
		if (sgroup.players[0].length() == 0 || sgroup.players[1].length() == 0) {
			continue;
		}
		//cerr << "scraping '"<<sgroup.players[0]<<"' vs '"<<sgroup.players[1]<<"'\n";

		//DEBUGOUTMARKER(WARN);
		//DEBUGOUT(ERR, vars1.size(),"");
		//DEBUGOUT(ERR, vars2.size(),"");
		sgroup.races[0] = ToLower(vars1[4])[0];
		sgroup.races[1] = ToLower(vars2[4])[0];
		//DEBUGOUTMARKER(WARN);
		//sgroup.timestamp=Timestamp(split[i+2]);

		int score1 = ToInt(vars1[8]);
		int score2 = ToInt(vars2[8]);
		//DEBUGOUTMARKER(WARN);
		if (vars1.size()>10 && vars1[10].length()) {
			sgroup.winner_scores[0] = score1;
			sgroup.loser_scores[0] = score2;
			sgroup.match_winners[0] = 1;
			sgroup.match_losers[0] = 2;
		}
		else if (vars2.size()>10 && vars2[10].length()) {
			sgroup.winner_scores[0] = score2;
			sgroup.loser_scores[0] = score1;
			sgroup.match_winners[0] = 2;
			sgroup.match_losers[0] = 1;
		}
		else {
			//cerr << "wtf\n";
		}
		smatches.push_back(sgroup);
		//DEBUGOUTMARKER(WARN);
	}
}

void ScrapeLiquipediaMatches(string &res, int t_id, vector<ScrapedGroup> &smatches)
{
	vector<string> textarea = SuperSplit(res.c_str(), "<textarea%>%</textarea>");
	vector<string> matches = SuperSplit_R(textarea[1].c_str(), "{{MatchList%");
	for (uint i = 1; i<matches.size(); i++) {
		vector<string> games = SuperSplit_R(matches[i].c_str(), "{{MatchMaps%\n}}%");
		string last_date = "";
		for (uint g = 1; g<games.size(); g += 2) {
			ScrapedGroup sgroup;
			sgroup.t_id = t_id;
			string player;
			player = SubStr(games[g].c_str(), "|player1=", "|", false);
			player = StripWhitespace(player);
			/*for(size_t s=player.length()-1;s<1000;s--) {
			if(player[s]!=' ') {
			player=player.substr(0, s+1);
			break;
			}
			}*/
			sgroup.players[0] = player;
			player = SubStr(games[g].c_str(), "|player2=", "|", false);
			player = StripWhitespace(player);
			/*for(size_t s=player.length()-1;s<1000;s--) {
			if(player[s]!=' ') {
			player=player.substr(0, s+1);
			break;
			}
			}*/
			sgroup.players[1] = player;
			sgroup.races[0] = SubStr(games[g].c_str(), "|player1race=", "|", false)[0];
			sgroup.races[1] = SubStr(games[g].c_str(), "|player2race=", "|", false)[0];
			string date;
			if (strstr(games[g].c_str(), "date=")) date = strstr(games[g].c_str(), "date=") + strlen("date=");
			if (date.length()>0 && date[0] != '\n' && date[0] != '|') {
				last_date = date;
			}
			sgroup.timestamp = Timestamp(last_date);
			string swinner = SubStr(games[g].c_str(), "winner=", "|", false);
			int winner = 0;
			sscanf(swinner.c_str(), "%d", &winner);
			sgroup.match_winners[0] = winner;
			if (winner == 1) {
				sgroup.match_losers[0] = 2;
			}
			else if (winner == 2) {
				sgroup.match_losers[0] = 1;
			}

			vector<string> scores = SuperSplit_R(games[g].c_str(), "|map%win=%|");
			for (uint s = 1; s<scores.size(); s += 3) {
				//cerr << scores[s] <<"\n";
				if (ToInt(scores[s]) == winner) {
					sgroup.winner_scores[g / 2]++;
				}
				else {
					sgroup.loser_scores[g / 2]++;
				}
			}
			smatches.push_back(sgroup);
		}
	}
}

void ScrapeLiquipediaGroups(string &res, int t_id, vector<ScrapedGroup> &groups)
{
	vector<string> textarea = SuperSplit(res.c_str(), "<textarea%>%</textarea>");
	vector<string> group_tables = SuperSplit_R(textarea[1].c_str(), "{{GroupTableStart%{{GroupTableEnd}}%{{MatchList%");
	for (uint i = 1; i < group_tables.size(); i += 3) {
		//cerr << i<<". ------------------\n"<<group_tables[i] << "\n--------------------\n"<<group_tables[i+2]<<"\n\n";
		ScrapedGroup sgroup;
		sgroup.t_id = t_id;
		string group = group_tables[i];
		sgroup.name = SubStr(group.c_str(), "| ", " |", false);
		string matches = group_tables[i + 2];
		string date = SubStr(group.c_str(), "date=", "}}", false);
		vector<string> slots = SuperSplit_R(group.c_str(), "{{GroupTableSlot|%{{%}}%}}%");
		//string players[4];
		//char races[4] = { 0, 0, 0, 0 };
		uint p = 0;
		for (uint i = 1; i < slots.size(); i += 4) {
			//cerr << "--------- " << slots[i]<<"---------------"<<slots[i + 1] << "-------" << slots[i + 2] << "\n";
			vector<string> vars;
			SplitBy(slots[i + 1].c_str(), '|', vars);
			for (uint v = 0; v < vars.size(); v++) {
				if (strstr(vars[v].c_str(), "race=")) {
					sgroup.races[p] = vars[v].back();
				}
				else if (strchr(vars[v].c_str(), '=') == NULL && vars[v] != "player") {
					sgroup.players[p] = StripWhitespace(vars[v]);
					if (strstr(slots[i].c_str(), "&lt;s>")) {
						cerr << vars[v] << " is a walkover!\n";
						sgroup.players[p] = "walkover";
					}
					const char *t = sgroup.players[p].c_str();
					const char *newend = strstr(t, " (");
					if (newend == NULL) newend = strstr(t, "(");
					if (newend) {
						uint newlen = (uint)(newend-t);
						cerr << "fixing player " << sgroup.players[p] << " to " << sgroup.players[p].substr(0, newlen)<<"\n";
						sgroup.players[p] = sgroup.players[p].substr(0, newlen);
					}
				}
			}
			p++;
		}
		for (uint i = 0; i < 4; i++) {
			string race = string(&sgroup.races[i], 1);
			//cerr << sgroup.players[i] << " " << race << "\n";
		}
		//cerr << date << "\n\n\n";
		if (strstr(date.c_str(), "\n|") == date.c_str()) {
			cerr << "no date yet!\n";
			//continue;
		}
		else if (date.length()) {
			sgroup.timestamp = Timestamp(date);
		}

		vector<string> games = SuperSplit_R(matches.c_str(), "{{MatchMaps%}}%");
		uint m = 0;
		for (uint g = 1; g<games.size(); g++) {
			//cerr << games[g]<<"\n\n";
			/*if (strstr(games[g].c_str(), "|date=")) {
			date = strstr(games[g].c_str(), "|date=") + strlen("|date=");
			auto t = Timestamp(date);
			if (sgroup.timestamp == 0 || sgroup.timestamp > t) sgroup.timestamp = t;
			}*/
			vector<string> split = SuperSplit(games[g].c_str(), "player1=%|%player2=%|");
			if (split.size() == 0) {
				//g--;
				continue;
			}
			assert(split.size() == 3);
			//cerr<<split[0]<<","<<split[1]<<","<<split[2]<<"\n";
			string splayer1 = split[0];
			string splayer2 = split[2];
			/*for(size_t s=splayer1.length()-1;s<1000;s--) {
			if(splayer1[s]!=' ') {
			splayer1=splayer1.substr(0, s+1);
			break;
			}
			}
			for(size_t s=splayer2.length()-1;s<1000;s--) {
			if(splayer2[s]!=' ') {
			splayer2=splayer2.substr(0, s+1);
			break;
			}
			}*/
			splayer1 = StripWhitespace(splayer1);
			splayer2 = StripWhitespace(splayer2);
			int player1 = 0;
			int player2 = 0;
			int winner = 0;
			const char *s = games[g].c_str();
			s = strstr(s, "winner=");
			if (s == NULL) s = strstr(games[g + 1].c_str(), "winner=");
			s += strlen("winner=");
			sscanf(s, "%d", &winner);
			//cerr << "winner == "<<winner<<"\n";

			for (uint p = 0; p<4; p++) {
				if (ToLower(sgroup.players[p]) == ToLower(splayer1)) {
					player1 = p + 1;
				}
				if (ToLower(sgroup.players[p]) == ToLower(splayer2)) {
					player2 = p + 1;
				}
			}
			if (winner == 1) {
				sgroup.match_winners[m] = player1;
				sgroup.match_losers[m] = player2;
				//cerr<<player1<<" beats "<<player2<<"\n";
			}
			else if (winner == 2) {
				sgroup.match_winners[m] = player2;
				sgroup.match_losers[m] = player1;
				//cerr<<player2<<" beats "<<player1<<"\n";
			}

			vector<string> scores = SuperSplit_R(s, "|map%win=% |%\n");
			for (uint s = 1; s<scores.size(); s += 4) {
				//cerr << scores[s] <<"\n";
				if (ToInt(scores[s]) == winner) {
					sgroup.winner_scores[g / 2]++;
				}
				else {
					sgroup.loser_scores[g / 2]++;
				}
			}
			m++;
		}
		groups.push_back(sgroup);
	}
}

void ScrapeLiquipediaGroupsDH(string &res, int t_id, vector<ScrapedGroup> &groups)
{
	vector<string> textarea = SuperSplit(res.c_str(), "<textarea%>%</textarea>");
	vector<string> group_tables = SuperSplit_R(textarea[1].c_str(), "{{GroupTableStart%{{GroupTableEnd}}%{{MatchList%");
	for (uint i = 1; i < group_tables.size(); i += 3) {
		//cerr << i<<". ------------------\n"<<group_tables[i] << "\n--------------------\n"<<group_tables[i+2]<<"\n\n";
		ScrapedGroup sgroup;
		sgroup.t_id = t_id;
		string group = group_tables[i];
		sgroup.name = SubStr(group.c_str(), "| ", " |", false);
		string matches = group_tables[i + 2];
		string date = SubStr(group.c_str(), "date=", "}}", false);
		vector<string> slots = SuperSplit_R(group.c_str(), "{{GroupTableSlot|%{{%}}%}}%");
		//string players[4];
		//char races[4] = { 0, 0, 0, 0 };
		uint p = 0;
		for (uint i = 1; i < slots.size(); i += 4) {
			//cerr << "--------- " << slots[i]<<"---------------"<<slots[i + 1] << "-------" << slots[i + 2] << "\n";
			vector<string> vars;
			SplitBy(slots[i + 1].c_str(), '|', vars);
			for (uint v = 0; v < vars.size(); v++) {
				if (strstr(vars[v].c_str(), "race=")) {
					sgroup.races[p] = vars[v].back();
				}
				else if (strchr(vars[v].c_str(), '=') == NULL && vars[v] != "player") {
					sgroup.players[p] = StripWhitespace(vars[v]);
					if (strstr(slots[i].c_str(), "&lt;s>")) {
						cerr << vars[v] << " is a walkover!\n";
						sgroup.players[p] = "walkover";
					}
				}
			}
			p++;
		}
		for (uint i = 0; i < 4; i++) {
			string race = string(&sgroup.races[i], 1);
			//cerr << sgroup.players[i] << " " << race << "\n";
		}
		//cerr << date << "\n\n\n";
		if (strstr(date.c_str(), "\n|") == date.c_str()) {
			cerr << "no date yet!\n";
			//continue;
		}
		else if (date.length()) {
			sgroup.timestamp = Timestamp(date);
		}

		vector<string> games = SuperSplit_R(matches.c_str(), "{{MatchMaps%}}%");
		uint cg = 0;
		for (uint g = 1; g<games.size(); g++) {
			//cerr << games[g]<<"\n\n";
			vector<string> split = SuperSplit(games[g].c_str(), "player1=%|%player2=%|");
			//assert(split.size()==3);
			if (split.size() != 3) continue;
			//cerr<<split[0]<<","<<split[1]<<","<<split[2]<<"\n";
			string splayer1 = split[0];
			string splayer2 = split[2];
			splayer1 = StripWhitespace(splayer1);
			splayer2 = StripWhitespace(splayer2);
			/*for (size_t s = splayer1.length() - 1; s<1000; s--) {
				if (splayer1[s] != ' ') {
					splayer1 = splayer1.substr(0, s + 1);
					break;
				}
			}
			for (size_t s = splayer2.length() - 1; s<1000; s--) {
				if (splayer2[s] != ' ') {
					splayer2 = splayer2.substr(0, s + 1);
					break;
				}
			}*/
			int player1 = 0;
			int player2 = 0;
			int winner = 0;
			const char *s = games[g].c_str();
			s = strstr(s, "winner=");
			s += strlen("winner=");
			sscanf(s, "%d", &winner);
			//cerr << "winner == "<<winner<<"\n";

			for (uint p = 0; p<4; p++) {
				if (ToLower(sgroup.players[p]) == ToLower(splayer1)) {
					player1 = p + 1;
				}
				if (ToLower(sgroup.players[p]) == ToLower(splayer2)) {
					player2 = p + 1;
				}
			}
			if (winner == 1) {
				sgroup.match_winners[cg] = player1;
				sgroup.match_losers[cg] = player2;
				//cerr<<player1<<" beats "<<player2<<"\n";
			}
			else if (winner == 2) {
				sgroup.match_winners[cg] = player2;
				sgroup.match_losers[cg] = player1;
				//cerr<<player2<<" beats "<<player1<<"\n";
			}

			vector<string> scores = SuperSplit_R(s, "|map%win=% |%\n");
			for (uint s = 1; s<scores.size(); s += 4) {
				//cerr << scores[s] <<"\n";
				if (ToInt(scores[s]) == winner) {
					sgroup.winner_scores[cg]++;
				}
				else {
					sgroup.loser_scores[cg]++;
				}
			}
			cg++;
		}
		groups.push_back(sgroup);
	}
}
