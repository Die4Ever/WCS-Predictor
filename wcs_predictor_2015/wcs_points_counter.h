
class PointOccurances
{
public:
	uint points;
	//uint times;
	//bool qualified;
	uint hits;
	uint misses;

	PointOccurances(uint Points, uint Hits, uint Misses)
	{
		points=Points;
		//times=Times;
		//qualified=Qualified;
		hits = Hits;
		misses = Misses;
	}
};

class PointsCounterSeason
{
public:
	
};


class PointsCounter
{
public:
	vector<vector<PointOccurances>> players;

	array<uint,96> point_cutoffs;

	PointsCounter() : players(MAX_PLAYER_ID)
	{
		for(auto &c : point_cutoffs) {
			c=0;
		}
	}

	int PointsToSlot(uint points)
	{
		return ((int)points-1800)/25;
	}

	uint SlotToPoints(int slot)
	{
		return slot*25+1800;
	}

	void AddCutoff(uint points)
	{
		for(int s=0;s<point_cutoffs.size();s++) {
			uint spoints=SlotToPoints(s);
			if(points<=spoints) {
				point_cutoffs[s]++;
			}
		}
	}

	void AddPlayerCountsOccurance(uint p_id, uint points, uint hits, uint misses)
	{
		for(auto &p : players[p_id]) {
			if(p.points==points) {
				p.hits += hits;
				p.misses += misses;
				return;
			}
		}
		players[p_id].push_back( PointOccurances(points,hits,misses) );
	}

	void ConsumeCounter(const PointsCounter &old)
	{
		for(int s=0;s<point_cutoffs.size();s++) {
			point_cutoffs[s]+=old.point_cutoffs[s];
		}
		for(uint p=0;p<MAX_PLAYER_ID;p++) {
			for(auto &points : old.players[p]) {
				AddPlayerCountsOccurance(p,points.points, points.hits, points.misses);
			}
		}
	}

	void Sort()
	{
		for(auto &p : players) {
			sort(p.begin(), p.end(), [](const PointOccurances &a, const PointOccurances &b){
				return a.points>b.points;
			});
		}
	}

	void GetStats(uint p_id, uint &min, uint &max, uint &mean, uint &median, uint &mode, uint &min_qualify, uint &max_not_qualify, uint &afk_times_top, uint &afk_samps)
	{
		min=987654321;
		max=0;
		mean=0;
		median=0;
		mode=0;
		uint mode_times=0;
		min_qualify=987654321;
		max_not_qualify=0;

		uint total_hits=0;
		uint total_qualified_hits=0;
		uint total_unqualified_hits=0;
		unsigned __int64 total=0;
		auto &points = players[p_id];
		for(auto &p : points) {
			uint times = p.hits + p.misses;
			total_hits+=times;
			total_qualified_hits += p.hits;
			if (p.points<min_qualify && p.hits>0) min_qualify = p.points;
			if (p.points>max_not_qualify && p.misses>0) max_not_qualify = p.points;
			/*if(p.qualified) {
				total_qualified_hits+=p.times;
				if(p.points<min_qualify) min_qualify=p.points;
			} else {
				total_unqualified_hits+=p.times;
				if(p.points>max_not_qualify) max_not_qualify=p.points;
			}*/


			total+=(unsigned __int64)p.points*(unsigned __int64)times;
			if(p.points>max) max=p.points;
			if(p.points<min) min=p.points;
			if(times>mode_times) {
				mode_times=times;
				mode=p.points;
			}
		}
		mean=(uint)(unsigned __int64)(total/(unsigned __int64)total_hits);

		uint counter=0;
		for(auto &p : points) {
			counter+=p.hits+p.misses;
			if(counter>total_hits/2) {
				median=p.points;
				break;
			}
		}

		afk_times_top = 0;
		afk_samps = 0;
		for (auto &p : points) {
			if (p.points == min) {
				afk_samps += p.hits+p.misses;
				afk_times_top += p.hits;
			}
		}
	}

	void Save(MysqlCon &con, unsigned __int64 sim_id DEBUGARGSDEF)
	{
		con.delete_query(("delete from players_wcs_points where sim_id="+ToString(sim_id)).c_str() DEBUGARGS);
		Sort();
		uint min, max, mean, median, mode, min_qualify, max_not_qualify, afk_times_top, afk_samps;
		string query = "insert into players_wcs_points (sim_id,season,p_id,min,max,mean,median,mode,min_qualify,max_not_qualify,afk_times_top,afk_samps) values ";
		string values;
		for(uint p_id=0;p_id<MAX_PLAYER_ID;p_id++) {
			if(players[p_id].size()) {
				GetStats(p_id, min, max, mean, median, mode, min_qualify, max_not_qualify, afk_times_top, afk_samps);
				values += "(" + ToString(sim_id) + ",0," + ToString(p_id) + "," + ToString(min) + "," + ToString(max) + "," + ToString(mean) + "," + ToString(median) + "," + ToString(mode) + "," + ToString(min_qualify) + "," + ToString(max_not_qualify) + "," + ToString(afk_times_top) + "," + ToString(afk_samps) + "),";
			}
		}
		if(values.length()) {
			values = values.substr(0, values.length() - 1);
			query += values;
			con.update(query.c_str() DEBUGARGS);
		}

		con.delete_query(("delete from point_cutoffs where sim_id="+ToString(sim_id)).c_str() DEBUGARGS);
		string cquery = "insert into point_cutoffs (sim_id, points, times_top) values ";
		string cvalues;
		for(uint s=0;s<point_cutoffs.size();s++) {
			cvalues += "("+ToString(sim_id)+","+ToString(SlotToPoints(s))+","+ToString(point_cutoffs[s])+"),";
		}
		if(values.length()) {
			cvalues = cvalues.substr(0, cvalues.length() - 1);
			cquery += cvalues;
			con.update(cquery.c_str() DEBUGARGS);
		}
	}
};
