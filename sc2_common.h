using std::abs;

const uint TOP_NUM_QUALIFY = 16;
const uint MAX_PLAYER_ID = 12000;
const uint MAX_TEAM_ID = 500;

#ifdef SIMULATOR
const size_t MAX_EVENTS_GROUP = 3;
#elif WEBSITE
const size_t MAX_EVENTS_GROUP = 8;
#endif

const unsigned __int64 EVG_ID_MULT = 40000;

const uint EVENTS_HASH_SIZE = 512;
const uint EVGS_HASH_SIZE = 4;//256;//256?//low for testing

const uint MAX_PLAYER_EVENTS = 80;
const uint MAX_PLAYER_EVENT_GROUPS = 192;
const uint SIM_EVENTS_BLOOM_SIZE = 16;//16;//maybe should be 8?
const size_t PRELIM_COUNT_DIVISOR = 10;//25
const size_t GROUP_EVENTS_DIVISOR = 1;//4;//4? 1 is probably overkill?
const size_t EVENTS_DIVISOR = 1;//4? 1 is probably overkill?

const double MINCHECKEVENTS = 0.0001;//0.01%//1%
const double MINEVENTSCORE = 0.0001;//0.006, 0.001//maybe I need to mess with this value some more
const double MINGLOBALEVENTRESOLUTION = 0.0005;//0.05%
const double MINEVENTRESOLUTION = 0.00005;//0.005%
const double MAXEVENTPROB = 1.0-MINEVENTRESOLUTION;
const double MINEVENTPROB = MINEVENTRESOLUTION;

const uint SPECIAL_EVENT = 200;
const uint UPCOMING_WIN = 255;
const uint UPCOMING_LOSE = 254;
const uint QUALIFY_BLIZZCON = 253;
//const uint NO_PLACEHOLDERS = 253;

const uint NORMAL_EVG_TYPE = 1;
const uint OR_EVG_TYPE = 2;
const uint NEG_EVG_TYPE = 3;
const uint NEG_AND_EVG_TYPE = 4;

uint Timestamp(string date)
{
	if (date.length() == 0) {
		cerr << "empty timestamp!\n";
		return 0;
	}
	tm t;
	t.tm_sec = 0;
	memset(&t, 0, sizeof(t));
	const char *months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	const char *s = date.c_str();
	if (*s == ' ') s++;
	const char *e = strchr(s, ' ');
	for (int month = 0; month<12; month++) {
		if (strstr(s, months[month])) {
			t.tm_mon = month;
			break;
		}
	}
	s = e + 1;
	e = strchr(s, ',');
	if (e == NULL) {
		cerr << "invalid date format " << date << "\n";
		return 0;
	}
	sscanf(s, "%d", &t.tm_mday);
	s = e + 2;
	sscanf(s, "%d", &t.tm_year);
	t.tm_year -= 1900;
	e = strstr(s, " - ");
	if (e) {
		s = e + 3;
	}
	else {
		e = strchr(s, ' ');
		s = e + 1;
	}
	if (e == NULL) {
		/*t.tm_hour=12;
		t.tm_min=0;
		uint timestamp=(uint)mktime(&t);
		return timestamp;*/
		s = "7:00 /CDT";
		cerr << "timestamp only found date! using 7am as default!\n";
	}
	if (*s == '0') s++;
	e = strchr(s, ':');
	sscanf(s, "%d", &t.tm_hour);
	s = e + 1;
	if (e == NULL) {
		t.tm_hour = 12;
		t.tm_min = 0;
		uint timestamp = (uint)mktime(&t);
		return timestamp;
	}
	if (*s == '0') s++;
	e = strchr(s, '/');
	sscanf(s, "%d", &t.tm_min);
	s = e + 1;
	uint timestamp = (uint)mktime(&t);
	e = strchr(s, '}');
	string timezone = string(s, 3);
	if (e) timezone = string(s, e - s);

	if (timezone == "KST") {
		timestamp -= 3600 * 15;
	}
	else if (timezone == "CET") {
		timestamp -= 3600 * 7;
	}
	else if (timezone == "EST") {
		timestamp -= 3600;
	}
	else if (timezone == "EET") {
		timestamp -= 3600 * 8;
	}
	else if (timezone == "BRST" || timezone == "BRS") {
		timestamp -= 3600 * 3;
	}
	else if (timezone == "PST") {
		timestamp += 3600 * 2;
	}
	else if (timezone == "PDT") {
		timestamp += 3600 * 1;
	}
	else if (timezone == "CDT") {
		timestamp -= 3600 * 1;
	}
	else if (timezone == "CST") {
		timestamp -= 3600 * 14;
	}
	else if (timezone == "CEST" || timezone == "CES") {
		timestamp -= 3600 * 8;
	}
	else if (timezone == "SGT") {
		timestamp -= 3600 * 13;
	}
	else if (timezone == "EES" || timezone == "EEST") {
		timestamp -= 3600 * 9;
	}
	else if (timezone == "EDT") {
		timestamp -= 3600 * 2;
	}
	else if (timezone == "BST") {
		timestamp -= 3600 * 7;
	}
	else if (timezone == "HKT") {
		timestamp -= 3600 * 14;
	}
	else if (timezone == "TST") {
		timestamp -= 3600 * 14;
	}
	else if (timezone == "MSK") {
		timestamp -= 3600 * 10;
	}
	else if (timezone == "AED" || timezone == "AEDT") {
		timestamp -= 3600 * 17;
	}
	else if (timezone == "AEST") {
		timestamp -= 3600 * 16;
	}
	else if (timezone == "GMT") {
		timestamp -= 3600 * 6;
	}
	else if (timezone == "CT") {
		//nothing?
	}
	else {
		cerr << "\nWTF timezone is " << date << " (" << timezone << ")??\n";
		throw rayexception("bad timezone!" DEBUGARGS);
	}
	//FL timezone
	//timestamp+=3600;
	return timestamp;
}

DefFixedStringType(PlayerName, 32, );
DefFixedStringType(TeamName, 32, );
DefFixedStringType(RoundName, 32, );
DefFixedStringType(TournamentName, 32, );

const char R = 0;
const char P = 1;
const char T = 2;
const char Z = 3;

class Player
{
public:
	int ratings[4];
	char race;
	uint player_id;
	char country[2];
	int wcs_points;
	int wcs_wcs_points;
	int team_id;
	char headband;
	uint league;
	int last_period;

	PlayerName name;
	vector<PlayerName> alternate_names;

	Player()
	{
		player_id = 0;
		ratings[0] = ratings[1] = ratings[2] = ratings[3] = 10000;//make them godmode so it stands out?
		wcs_points = 0;
		wcs_wcs_points = 0;
		//times_top = 0;
		race = R;
		headband = 0;
		memcpy(country, "kr", 2);
		team_id = 0;
		league = 0;
		last_period = 0;
	}

	string sCountry()
	{
		if (country[0]) return string(country, 2);
		return string("");
	}

	bool MatchName(string test)
	{
		if (this == NULL) {
			return (test == "" || test == "TBD");
		}
		string a = ToLower(name.ToString());
		string b = ToLower(test);
		if (a != b) {
			for (size_t i = 0; i<alternate_names.size(); i++) {
				a = ToLower(alternate_names[i].ToString());
				if (a == b) return true;
			}
		}
		return a == b;
	}
};

/*class SmallPlayer
{
public:
	int ratings[4];
	char race;
	char region;
	short player_id;

	SmallPlayer &operator->()
	{
		return *this;
	}
};*/

class Team
{
public:
	int team_id;
	TeamName name;

	Team()
	{
		team_id = 0;
	}
};

const uint scraped_group_matches = 32;
class ScrapedGroup
{
public:
	uint timestamp;
	array<string, 8> players;
	char races[8];
	int match_winners[scraped_group_matches];//index into players array for the winner?
	int match_losers[scraped_group_matches];
	int winner_scores[scraped_group_matches];
	int loser_scores[scraped_group_matches];
	int t_id;
	string name;

	ScrapedGroup()
	{
		timestamp = 0;
		t_id = 0;
		for (uint i = 0; i<8; i++)
			races[i] = 0;
		for (uint i = 0; i<scraped_group_matches; i++)
			match_winners[i] = match_losers[i] = winner_scores[i] = loser_scores[i] = 0;
	}
};

class PreviousMatch
{
public:
	uint timestamp;
	uint winner_id;
	uint loser_id;
	int score_winner;
	int score_loser;
	uint t_id;
	uint round;

	PreviousMatch() {}

	PreviousMatch(Player *winner, Player *loser, int matchnum, ScrapedGroup &info, uint roundof)
	{
		winner_id = winner->player_id;
		loser_id = loser->player_id;
		score_winner = info.winner_scores[matchnum];
		score_loser = info.loser_scores[matchnum];
		timestamp = info.timestamp;
		t_id = info.t_id;
		round = roundof;
	}
};

class Headband
{
public:
	int headband;
	uint defender;
	uint challenger;
	int defended;
	uint played_time;
	uint t_id;
	int current;//only used on website side

	Headband(uint challenger_id, uint defender_id, int defender_headband, int challenger_headband, uint tournament_id, uint timestamp, int defender_won)
	{
		if (BDEBUG) {
			if (defender_headband == 0) throw rayexception("defender headband is 0?" DEBUGARGS);
			if (defender_headband >= challenger_headband && challenger_headband != 0) throw rayexception("defender has the >= headband?" DEBUGARGS);
			if (defender_headband == 1 && challenger_headband == 0) throw rayexception("defender has #1 and challenger does not have a headband?" DEBUGARGS);
		}
		headband = defender_headband;
		defender = defender_id;
		challenger = challenger_id;
		defended = defender_won;
		played_time = timestamp;
		t_id = tournament_id;
		current = 0;
	}

	Headband(uint challenger_id, uint defender_id, int defender_headband, uint tournament_id, uint timestamp, int defender_won)
	{
		if (BDEBUG) {
			if (defender_headband == 0) throw rayexception("defender headband is 0?" DEBUGARGS);
		}
		headband = defender_headband;
		defender = defender_id;
		challenger = challenger_id;
		defended = defender_won;
		played_time = timestamp;
		t_id = tournament_id;
		current = 0;
	}

	Headband(PreviousMatch &prevmatch, uint challenger_id, uint defender_id, int defender_headband, int challenger_headband)
	{
		if (BDEBUG) {
			if (defender_headband == 0) throw rayexception("defender headband is 0?" DEBUGARGS);
			if (defender_headband >= challenger_headband && challenger_headband != 0) throw rayexception("defender has the >= headband?" DEBUGARGS);
			if (defender_headband == 1 && challenger_headband <= 1) throw rayexception("defender has #1 and challenger does not have #x?" DEBUGARGS);
			if (defender_id != prevmatch.winner_id && defender_id != prevmatch.loser_id) throw rayexception("defender_id not found in prevmatch?" DEBUGARGS);
			if (challenger_id != prevmatch.winner_id && challenger_id != prevmatch.loser_id) throw rayexception("challenger_id not found in prevmatch?" DEBUGARGS);
		}
		headband = defender_headband;
		defender = defender_id;
		challenger = challenger_id;
		defended = 0;
		if (defender_id == prevmatch.winner_id)
			defended = 1;
		played_time = prevmatch.timestamp;
		t_id = prevmatch.t_id;
		current = 0;
	}

};



class Event
{
public:
	//uint h;
	union {
		uint uu;
		struct {
			unsigned short p_id;
			byte t_id;
			byte placing;
		};
	};

	//do I need to store the season in the event? can maybe just look up the season from the tournament using t_id if I need

	Event()
	{
		p_id = 0;
	}

	Event(uint TID, uint PID, uint PLACING)
	{
		assert(PID < (1 << 16));
		assert(PID < MAX_PLAYER_ID);
		assert(PLACING < (1 << 8));
		assert(TID < (1 << 8));
		p_id = PID;
		t_id = TID;
		placing = PLACING;
		//h = Hash(*this);
	}

	static unsigned int Hash(const Event &k)
	{
		//return k.uu+k.p_id;

		unsigned int h = 0;
		h += (uint)k.t_id * 17u;
		h += (uint)k.p_id * 777u;
		h += (uint)k.placing * 797977u;
		//h += k.uu;
		return h;
	}

	const unsigned int Hash()
	{
		//return h;
		return Hash(*this);
	}

	const int SortComp(Event* objB)
	{
		return uu != objB->uu;
		return /*h != objB->h ||*/ t_id != objB->t_id || p_id != objB->p_id || placing != objB->placing;
	}

	const int SortComp(Event objB)
	{
		return uu != objB.uu;
		return /*h != objB.h ||*/ t_id != objB.t_id || p_id != objB.p_id || placing != objB.placing;
	}
};

int CompEvents(const Event &a, const Event &b)
{
	//if (a.uu == b.uu) return 0;
	if (a.uu > b.uu) return 1;
	if (a.uu < b.uu) return -1;
	return 0;

	//if (a.h > b.h) return 1;
	//if (a.h<b.h) return -1;
	if (a.t_id == b.t_id && a.p_id == b.p_id && a.placing == b.placing) return 0;
	if (a.p_id>b.p_id) return 1;
	if (a.p_id<b.p_id) return -1;
	if (a.placing>b.placing) return 1;
	if (a.placing<b.placing) return -1;
	if (a.t_id>b.t_id) return 1;
	if (a.t_id < b.t_id) return -1;
	return 0;
}

double ScoreChances(double prob, double newchances, double ev_prob)
{
	double diff = abs(newchances - prob);
	//double score = diff*(ev_prob*0.8 + 0.2);
	ev_prob=0.6-abs(ev_prob-0.6);
	double tev_prob = (0.05+ev_prob)/1.25;
	//if(ev_prob<MINEVENTPROB) tev_prob=ev_prob*0.001;
	double score = diff*tev_prob+tev_prob*0.01;
	return score;
}

double ModScoreEvent(uint p, double score, size_t s, const Event &e)
{
	//if(e.t_id==100 && e.p_id!=p && e.placing!=2) score=MINEVENTSCORE*0.001;
	if(s==1) {
		if(e.p_id==p) {
			score+=score*1.25;
			//if(e.placing<200 && e.t_id==100) score+=0.5;
			if(e.placing==1) score+=score*1.0+0.05;
			else if(e.placing==2) score+=score*0.4+0.0005;
			else if(e.placing>=SPECIAL_EVENT) {
				score+=score*1.1+0.5;
			}
		} else if (s == 1 && (e.placing == UPCOMING_WIN || e.placing == UPCOMING_LOSE)) {
			score += score*0.1;
		}
	} else if(e.p_id==p) {
		score*=1.5;
	}
	if(e.p_id!=p) {
		score *= 0.1;
	}
	//if(s==2 && e.placing<=2 && e.t_id==100 && (e.placing==2 || e.p_id==p)) score+=0.2;//this should make it so grouped finalists for blizzcon are shown, and it will only show up for the player that got the 1st place
	if(e.placing<SPECIAL_EVENT && e.t_id>100) score*=0.25;
	score += ((double)(e.placing == UPCOMING_WIN))*0.0001;//<----------------
	if (s == 1 && e.p_id == p && e.placing == QUALIFY_BLIZZCON) score = -10.0;
	return score;
}

double ModPurgeScoreEvent(uint p, double score, size_t s, const Event &e, double ev_prob)
{
	if (e.p_id == p && s==1) {
		score *= 1.5;
		score += MINEVENTSCORE+0.075;// *0.8;
		if(e.placing==1 && ev_prob>0.0005) score += 0.1;
		else if(e.placing==2 && ev_prob>0.005) score += 0.09;
		else if( e.placing>=SPECIAL_EVENT && ev_prob>0.005) {
			score += 0.25;
		}
	}/* else if(e.placing==1 || e.placing==2) {
		score += score*0.1;//group with 1sts and 2nds more than others, what about upcomings?
	}*/
	if (e.p_id == p) score *= 1.5;
	else score *= 0.1;//necessary?
	//if (s > 1 && e.p_id != p) score *= 0.5;//stop all this madness!
	if(e.placing<SPECIAL_EVENT && e.t_id>100) score*=0.75;
	score += ((double)(e.placing == UPCOMING_WIN))*0.0001;
	//if(e.p_id==p && s>1) score *= 1.01;//1% gain for each additional self event? try to get some more multiples in there?
	if (s == 1 && e.p_id == p && e.placing == QUALIFY_BLIZZCON) score = -10.0;
	return score;
}

double FinalizeScore(size_t s, double score, double newchances, double ev_prob)
{
	double ds = (double)s;
	double ds2 = pow(ds, 6.0);
	score = (score / log2(ds2+4.0))*3.0;
	if (s>1 && ev_prob > MAXEVENTPROB) score = -1.0;//this should help with branches, and it shouldn't normally be needed since "always" events are never inserted
	if (ev_prob*ds < MINEVENTPROB) score *= ev_prob;
	return score;
}

class EventCounter : public Event
{
public:
	uint hits;//times this event happened and the current player qualified
	uint misses;//times this event happened and the current player didn't qualify

	EventCounter(Event &source) : Event(source)
	{
		hits = misses = 0;
	}

	EventCounter() : Event()
	{
		hits = misses = UINT_MAX;
	}

	double ScoreEvent(uint sims, double prob, uint p) const
	{
		//size_t s = 1;
		if(hits+misses==0) return 0.0;
		double newchances = (double)hits / (double)(hits + misses);
		double ev_prob = (double)(hits + misses) / (double)sims;
		double score = ScoreChances(prob,newchances,ev_prob);

		score = ModScoreEvent(p,score,1,*this);

		score = FinalizeScore(1, score, newchances, ev_prob);
		return score;
	}

	double PurgeScoreEvent(uint sims, double prob, uint p) const
	{
		//size_t s = 1;
		double newchances = (double)hits / (double)(hits + misses);
		double ev_prob = (double)(hits + misses) / (double)sims;
		double score = ScoreChances(prob,newchances,ev_prob);

		score = ModPurgeScoreEvent(p,score,1,*this, ev_prob);

		score = FinalizeScore(1, score, newchances, ev_prob);
		return score;
	}

};

class EventGroup
{
public:
	uint hits;
	uint misses;
	array<EventCounter, MAX_EVENTS_GROUP> events;
	size_t isize;
	unsigned __int64 evg_id;

	EventGroup()
	{
		evg_id = 0;
		isize = hits = misses = 0;
	}

	void AddEvent(EventCounter ev)
	{
		size_t s = size();
		AddEvent(ev, s);
	}

	void AddEvent(EventCounter ev, size_t s)
	{
		assert(s<MAX_EVENTS_GROUP);
		for (size_t i = 0; i<s; i++) {
			if (ev.SortComp(events[i]) == 0) return;
		}
		events[s] = ev;
		isize++;

		std::sort(events.begin(), events.end(), [](const EventCounter &a, const EventCounter &b)
		{
			if (a.hits + a.misses != b.hits + b.misses) return a.hits + a.misses < b.hits + b.misses;
			if (a.p_id != b.p_id) return a.p_id > b.p_id;
			if (a.t_id != b.t_id) return a.t_id > b.t_id;
			if (a.placing != b.placing) return a.placing > b.placing;
			return a.t_id > b.t_id;
		});
	}

	size_t size() const
	{
		return isize;
		size_t s = 0;
		for (; s < MAX_EVENTS_GROUP; s++) {
			if (events[s].p_id == 0) break;
		}
		return s;
	}

	static unsigned int Hash(EventGroup &k)
	{
		uint h = 0;
		uint s = (uint)k.size();
		for (uint i = 0; i < s; i++) {
			h += k.events[i].Hash()*(i + 1) * 73;
		}
		return h;
	}

	const unsigned int Hash()
	{
		return Hash(*this);
	}

	const int SortComp(EventGroup &objB)
	{
		size_t sA = size();
		size_t sB = objB.size();
		if (sA != sB) return 1;
		for (size_t a = 0; a < sA; a++) {//if I sort the events then it's linear instead of exponential on compare
			if (events[a].SortComp(objB.events[a]) != 0) {
				return 1;
			}
		}
		return 0;
	}

	const int SuperSubSet(EventGroup &objB)
	{//1 for I'm the superset, -1 for I'm the subset, 0 otherwise
		size_t sA = size();
		size_t sB = objB.size();
		if (hits != objB.hits || misses != objB.misses || sA == sB) {
			return 0;
		}
		size_t matches = 0;
		for(size_t b=0;b<sB;b++) {
			for(size_t a=0;a<sA;a++) {
				if(objB.events[b].SortComp(events[a])==0) {
					matches++;
					break;
				}
			}
		}
		if (sA > sB) {//check if I'm a superset of this
			if (matches == sB) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if (matches == sA) {
				return -1;
			}
			else {
				return 0;
			}
		}
		return 0;
	}

	const int LooseSuperSubSet(EventGroup &objB)
	{//1 for I'm the superset, -1 for I'm the subset, 0 otherwise
		size_t sA = size();
		size_t sB = objB.size();
		if (/*hits != objB.hits || misses != objB.misses ||*/ sA == sB) {
			return 0;
		}
		size_t matches = 0;
		for(size_t b=0;b<sB;b++) {
			for(size_t a=0;a<sA;a++) {
				if(objB.events[b].SortComp(events[a])==0) {
					matches++;
					break;
				}
			}
		}
		if (sA > sB) {//check if I'm a superset of this
			if (matches == sB) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if (matches == sA) {
				return -1;
			}
			else {
				return 0;
			}
		}
		return 0;
	}

	double ScoreEvent(uint sims, double prob, uint p) const
	{
		if(hits+misses==0) return 0.0;
		size_t s = size();
		double newchances = (double)hits / (double)(hits + misses);
		double ev_prob = (double)(hits + misses) / (double)sims;
		double score = ScoreChances(prob,newchances,ev_prob);

		for (size_t i = 0; i<s; i++) {
			auto &e = events[i];
			score = ModScoreEvent(p,score,s,e);
		}
		score = FinalizeScore(s, score, newchances, ev_prob);
		/*if(s==2 && ((events[0].t_id==100) + (events[1].t_id==100))==1) {
			score=0.0;
		} else if(s==2 && ((events[0].t_id==100) + (events[1].t_id==100))==2) {
			if( ((events[0].placing==1 && events[0].p_id==p) + (events[0].placing==2) + (events[1].placing==1 && events[1].p_id==p) + (events[1].placing==2)) == 2 ) {
				score+=score*2.0+0.5;
			}
		}*/
		return score;
	}

	double PurgeScoreEvent(uint sims, double prob, uint p) const
	{
		size_t s = size();
		double newchances = (double)hits / (double)(hits + misses);
		double ev_prob = (double)(hits + misses) / (double)sims;
		double score = ScoreChances(prob,newchances,ev_prob);

		for (size_t i = 0; i<s; i++) {
			auto &e = events[i];
			//score = ModScoreEvent(p,score,s,e);
			score = ModPurgeScoreEvent(p,score,s,e, ev_prob);
		}
		score = FinalizeScore(s, score, newchances, ev_prob);
		/*if(s==2 && ((events[0].t_id==100) + (events[1].t_id==100))==1) {
			score=0.0;
		} else if(s==2 && ((events[0].t_id==100) + (events[1].t_id==100))==2) {
			if( ((events[0].placing==1 && events[0].p_id==p) + (events[0].placing==2) + (events[1].placing==1 && events[1].p_id==p) + (events[1].placing==2)) == 2 ) {
				score+=score*2.0+100.5;
			}
		}*/
		return score;
	}
};

class UpcomingMatch
{
public:
	uint timestamp;
	uint players[8];
	uint t_id;
	uint id;
	uint round;
	bool estimated_time;
	//int season;
	static uint last_id;

	UpcomingMatch()
	{
		memset(this, 0, sizeof(*this));
		id=last_id++;
		estimated_time=false;
	}
};
uint UpcomingMatch::last_id=1;

class WebEventGroup : public EventGroup
{
public:
	uint pid;
	uint type;
	WebEventGroup() {
		pid=0;
		type=1;
	}
};
