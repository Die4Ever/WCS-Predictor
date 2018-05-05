
int last_period = 0;
vector<Player> players;
vector<Player*> good_players;
vector<Player*> good_koreans;
vector<Team> teams;

Player &GetPlayerById(uint id DEBUGARGSDEF)
{
	assert(id < MAX_PLAYER_ID);
	if (id < players.size() && players[id].player_id == id)
		return players[id];

	cerr << id << " not in the proper slot?\n";
	for (auto &p : players) {
		if (p.player_id == id) {
			return p;
		}
	}
	cout << id << " not found!\n";
	cerr << id << " not found!\n";
	throw badplayer(ToString(id) DEBUGARGS);
}

string PlayerCorrection(Player &p)
{
	string pteamname;
	if (p.team_id) pteamname = teams[p.team_id].name.ToString();
	string srace = "Random";
	if (p.race == P) srace = "Protoss";
	if (p.race == T) srace = "Terran";
	if (p.race == Z) srace = "Zerg";
	string ret = pteamname + (pteamname.length()>0?" ":"") + p.name.ToString() + " as " + srace + " from " + p.sCountry() + ", ID == " + ToString(p.player_id);
	return ret;
}

Player &SearchPlayer(const char *cname DEBUGARGSDEF, bool suggest = false)
{
	Player *p=NULL;
	string name=ToLower(cname);
	vector<string> split;
	SplitBy(name.c_str(),';', split);
	string ta;
	string a;
	if(split.size()==1) a=split[0];
	else {
		ta=split[0];
		a=split[1];
	}
	const char *teamname=ta.c_str();
	if(ta.length()==0) teamname=NULL;

	string country="";
	if(split.size()>3) {
		country=split[3];
		if(split[3]=="?") country="";
	}
	char race=100;
	if(split.size()>2) {
		if(split[2]=="p") race=P;
		if(split[2]=="z") race=Z;
		if(split[2]=="t") race=T;
		if(split[2]=="r") race=R;
		if(split[2]=="?") race=100;
	}
	bool multiple = false;
	for (auto &tp : players) {
		if (suggest && tp.MatchName(a)) {
			cerr << PlayerCorrection(tp) << "\n";
		}
		if(race<100 && tp.race!=race) continue;
		if(country.length() && tp.sCountry()!=country) continue;

		if( tp.MatchName(a) ) {
			if (tp.team_id == 0 && (teamname == NULL || ta=="?")) {
				if(p==NULL) {
					p=&tp;
				} else {
					throw badplayer(string(name) DEBUGARGS);
				}
			} else if (tp.team_id && teamname) {
				string tb = ToLower(teams[tp.team_id].name.c_str());
				if (ta == tb || ta=="?") {
					if(p==NULL) {
						p=&tp;
					} else {
						//throw badplayer(string(name) DEBUGARGS);
						if (multiple == false) {
							cerr << "multiple players found for " << cname << "\n";
							cerr << PlayerCorrection(*p) <<"\n";
						}
						cerr << PlayerCorrection(tp) <<"\n";
						multiple = true;
					}
				}
			}
		}
	}

	//assert(p!=NULL);
	if (multiple && BDEBUG && BENCHMARK) return players[0];
	if (multiple) throw badplayer((name) DEBUGARGS);
	if (p == NULL) {
		if (suggest == false) {
			cerr << "\ncould not find " << name << "\n";
			DEBUGOUTSTACK(TRACE, name, "could not find player");
			cerr << "trying to find suggestions\n";
			SearchPlayer(cname DEBUGARGS, true);
		}
		if(BDEBUG && BENCHMARK) return players[0];
		if (BDEBUG) return players[0];
		throw badplayer(string(name) DEBUGARGS);
	}

	string pteamname="";
	if(p->team_id) pteamname=teams[p->team_id].name.ToString();
	if( ta=="?" || race!=p->race || (country.length()==0 || country!=p->sCountry()) || ta!=ToLower(pteamname) ) {
		if (p->player_id) cerr << PlayerCorrection(*p) << "\n";
		/*string srace="Random";
		if(p->race==P) srace="Protoss";
		if(p->race==T) srace="Terran";
		if(p->race==Z) srace="Zerg";
		cerr << pteamname <<" "<< p->name<<" as "<<srace<<" from "<<p->sCountry()<<", ID == "<<p->player_id<<"\n";
		DEBUGOUTSTACK(TRACE, name, "corrected player");
		race=100;*/
	}
	return *p;
}

double GlobalMatchPredict(Player &A, Player &B, int best_of)
{
	assert(A.player_id==0 || A.player_id!=B.player_id);
	if(USE_ALIGULAC==0 && (A.player_id!=0 && B.player_id!=0)) {
		return 0.5;
	}
	double ratingA = (double)A.ratings[B.race];
	double ratingB = (double)B.ratings[A.race];
	double power = (double)(best_of + 2.0) / 2.0;
	double c = pow(ratingA, power) / (pow(ratingA, power) + pow(ratingB, power));
	return c;
}

string StripAllWhitespace(const string &s)
{
	string r;
	r.reserve(s.capacity());
	for(size_t i=0;i<s.length();i++) {
		if(!Is_Whitespace(s[i])) {
			r+=s[i];
		}
	}
	return r;
}

string StripWhitespace(const string &s)
{
	size_t start=0;
	size_t end=s.length();
	for(size_t i=0;i<s.length();i++) {
		if(!Is_Whitespace(s[i])) {
			start=i;
			break;
		}
	}
	for(size_t i=s.length()-1;i<s.length();i--) {
		if(!Is_Whitespace(s[i])) {
			end=i+1;
			break;
		}
	}
	return s.substr(start, end-start);
}

void LoadAligulacPage(int period, int current_period, int page, atomic<int> &pages)
{
	string url = "http://aligulac.com/periods/" + ToString(period) + "/?page=" + ToString(page) + "&sort=&race=ptzrs&nats=all";
	cerr << "Loading aligulac period "<<period<<" page " << page << "/" << pages << "\n"<<url<<"\n";
	string res;
	vector<string> table;

	for(uint i=0;i<20 && table.size()!=3;i++) {
		bool readcache=false;
		if(BDEBUG || BENCHMARK) readcache=true;
		res = HttpRequest(url.c_str(), readcache, false);
		table = SuperSplit(res.c_str(), "<ul class=\"pagination\">%<table%>%</table>");
		if(table.size()!=3) {
			cerr << "table.size()=="<<table.size()<<"\n";
			RaySleep(1000*1000*10);
		}
	}
	if(table.size()!=3) {
		cerr << "table.size()=="<<table.size()<<", already tried a bunch of times!\n";
	}
	assert(table.size() == 3);

	if(page) {
		vector<string> page_buttons = SuperSplit(res.c_str(), "<ul class=\"pagination\">%</ul>%<ul class=\"pagination\">%</ul>");
		assert(page_buttons.size()==3);
		//cerr << page_buttons[2] << "\n";
		vector<string> page_buttons2 = SuperSplit_R(page_buttons[2].c_str(), "<li%>%</li>%");
		assert(page_buttons2.size()>2);
		vector<string> last_page_button = SuperSplit(page_buttons2[page_buttons2.size()-2].c_str(), "<a href=\"?page=%&");
		assert(last_page_button.size()==1);
		int old_pages=pages;
		pages = ToInt(last_page_button[0]);
		if(old_pages!=1 && pages!=old_pages) {
			cerr << "\nPage "<<page<<" found "<<ToInt(last_page_button[0])<<" pages, old_pages=="<<old_pages<<"\n\n";
		}
	}

	vector<string> rows = SuperSplit_R(table[2].c_str(), "<tr%>%</tr>%");
	//for(auto s:rows) cout << "\n------------------\n"<<s<<"\n----------------------\n\n";
	assert(rows.size() > 0);
	for (size_t r = 5; r < rows.size(); r += 3) {

		vector<string> cells = SuperSplit_R(rows[r].c_str(), "<td%>%</td>%");
		assert(cells.size() == 46);

		Player p;

		string country = SubStr(cells[8].c_str(), "<img src=\"http://img.aligulac.com/flags/", ".png", false);
		if (country.length() == 0) country = "  ";
		assert(country.length() == 2);
		memcpy(p.country, country.c_str(), 2);
		string race = SubStr(cells[11].c_str(), "<img src=\"http://img.aligulac.com/", ".png", false);
		assert(race.length() == 1);
		if (race == "T") p.race = T;
		else if (race == "P") p.race = P;
		else if (race == "Z") p.race = Z;
		else if (race == "S") p.race = R;
		else if (race == "R") p.race = R;
		else if (race == "") p.race = R;
		else
			throw rayexception((string("invalid race from aligulac == \"")+race+"\"").c_str() DEBUGARGS);

		p.player_id = ToUInt(SubStr(cells[14].c_str(), "<a href=\"/players/", "-", false).c_str());
		assert(p.player_id > 0);
		assert(p.player_id < MAX_PLAYER_ID);
		
		p.name = StripWhitespace(SubStr(cells[14].c_str(), "/\">", "</a>", false));
		p.team_id = ToUInt(SubStr(cells[17].c_str(), "<a href=\"/teams/", "-", false).c_str());
		assert(p.team_id < MAX_TEAM_ID);
		if (p.team_id) {
			string newteamname = StripWhitespace(SubStr(cells[17].c_str(), "/\">", "</a>", false));
			if (AFK == false || teams[p.team_id].team_id == 0) {
				teams[p.team_id].name = TeamName(newteamname.c_str());
				teams[p.team_id].team_id = p.team_id;
			}
		}

		p.ratings[R] = ToInt(cells[20].c_str());
		p.ratings[P] = ToInt(cells[26].c_str());
		p.ratings[T] = ToInt(cells[32].c_str());
		p.ratings[Z] = ToInt(cells[38].c_str());

		p.last_period=max(p.last_period,period);

		assert(p.ratings[R] > 100);
		assert(p.ratings[R] < 4000);
		assert(p.ratings[T]>100);
		assert(p.ratings[T] < 4000);
		assert(p.ratings[P]>100);
		assert(p.ratings[P] < 4000);
		assert(p.ratings[Z]>100);
		assert(p.ratings[Z] < 4000);
		assert(p.ratings[R]>100);
		assert(p.ratings[R] < 4000);

		if (AFK) {
			p.player_id = players[p.player_id].player_id;//this way new players are not added?
			p.name = players[p.player_id].name;
			p.team_id = players[p.player_id].team_id;
			memcpy(p.country, players[p.player_id].country, 2);
			p.race = players[p.player_id].race;
		}
		players[p.player_id] = p;
	}
}

void LoadAligulacPages(int period, int current_period DEBUGARGSDEF)
{
	atomic<int> threads;
	threads = 0;
	atomic<int> pages;
	pages = 40;
	//LoadAligulacPage(period, 1, pages);
	for (int p = 1; p <= pages; p++) {//load the players who are marked as inactive in the current period, this is a hack lol
		{
			while (threads>=8) RaySleep(1000);
			RaySleep(1000);
			threads++;
			thread([&threads, period, current_period, &pages, p]() {
				//int dpages = pages;
				LoadAligulacPage(period, current_period, p, pages);
				threads--;
			}).detach();
		}
	}
	while (threads>0) RaySleep(10000);
}

Player& MakeNewb(const char *teamname, const char *playername, string country, char race, uint player_id=1)
{
	string team_name="NULL";
	string A;
	if (teamname != NULL) {
		A = ToLower(teamname);
	}
	cerr << "\n\nLooking for newb "<<playername<<" on team "<<team_name<<"\n";
	try {
		//auto *p = &GetPlayerByName(playername, teamname DEBUGARGS);
		string steam=team_name;
		if(steam=="NULL") steam="";
		steam = "?";
		string srace="r";
		if(race==T) srace="t";
		if(race==P) srace="p";
		if(race==Z) srace="z";
		auto *p = &SearchPlayer( (steam+";"+string(playername)+";"+srace+";"+country).c_str() DEBUGARGS);
		if(p && p->player_id) {
			cerr << "Newb found\n";
			return *p;
		}
	} catch(...) {
	}
	cerr << "Newb not found, creating newb "<<playername<<" on team "<<team_name<<"\n";

	uint team_id=0;
	if (teamname != NULL) {
		for (auto &t : teams) {
			if (t.team_id == 0) continue;
			string B = ToLower(t.name.ToString());
			if (A == B) team_id = t.team_id;
		}
	}
	if(player_id!=1 && players[player_id].player_id!=0) {
		cerr << "wtf bad aligulac_id for MakeNewb\n";
		player_id=1;
	}
	for(uint i=player_id;i<players.size();i++) {
		auto &p = players[i];
		if(p.player_id!=0) continue;
		p.player_id=i;
		p.name=playername;
		int rating=999;
		if(country=="kr") rating=1199;
		p.ratings[R]=p.ratings[T]=p.ratings[P]=p.ratings[Z]=rating;
		p.race=race;
		p.team_id=team_id;
		if(country.length()==2)
			memcpy(p.country, country.c_str(), 2);
		p.last_period=90;
		cerr << "Newb "<<playername<<" on team "<<team_name<<" created with aligulac_id of "<<p.player_id<<"\n";
		return p;
	}
	cerr << "Not enough room for newb "<<playername<<" on team "<<team_name<<"!\n";
	return players[0];
}

void SavePlayers(MysqlCon &con);

void LoadPlayers(DEBUGARGDEF)
{
	players.clear();
	players.resize(MAX_PLAYER_ID);
	teams.clear();
	teams.resize(MAX_TEAM_ID);

	last_period = 0;
	{
		cout << "Connecting to mysql server...\n";
		MysqlCon con;
		con.ConnectToMysql("../sc2_mysqlcreds.txt");

		auto *dbres = con.select("select players.p_id, teams.team_id, players.name, teams.name, race, country, overall, vsP, vsT, vsZ, last_period from players left join teams using(team_id) where length(players.name)>0" DEBUGARGS);

		auto num_rows = mysql_num_rows(dbres);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			MYSQL_ROW row = mysql_fetch_row(dbres);
			uint p_id = ToUInt(row[0]);
			assert(p_id < players.size());
			if(!BENCHMARK && (ToInt(row[6])==800 || ToInt(row[6])==999 || ToInt(row[6])==1199) ) {
				continue;//skip generated newbs, so I can read them from aligulac and get their real ID
			}
			Player &p = players[p_id];
			p.player_id = p_id;
			p.team_id = ToUInt(row[1]);

			p.name = row[2];
			p.race = (int)ToInt(row[4]);

			if (row[5] && strlen(row[5]) == 2)
				memcpy(p.country, row[5], 2);
			else
				memcpy(p.country, "\0", 2);

			p.ratings[(int)R] = ToInt(row[6]);
			p.ratings[P] = ToInt(row[7]);
			p.ratings[T] = ToInt(row[8]);
			p.ratings[Z] = ToInt(row[9]);

			p.last_period = ToInt(row[10]);
			last_period = std::max<uint>(last_period, p.last_period);

			if (p.team_id) {
				teams[p.team_id].name = row[3];
				teams[p.team_id].team_id = p.team_id;
			}
		}
		con.free_result();
	}

	if /*(0) /*/(!BENCHMARK)
	{
		string res;
		int current_period = 0;
		for (uint i = 0; i < 50 && current_period == 0; i++) {
			res = HttpRequest("http://aligulac.com/periods/latest/", false, false);
			vector<string> split = SuperSplit(res.c_str(), "<title>List %</title>");
			if (split.size() == 1) {
				current_period = ToInt(split[0]);
			}
		}
		if(current_period<=1) {
			assert(0);
			cerr << "current_period=="<<current_period<<"\n";
			exit(1);
		}
		assert(current_period > 1);
		/*for(int i=current_period-8;i<current_period-1;i++) {
			LoadAligulacPages(i, current_period DEBUGARGS);
		}*/
		if(last_period<current_period) LoadAligulacPages(current_period - 1, current_period DEBUGARGS);
		LoadAligulacPages(current_period, current_period DEBUGARGS);
	}

	/*players[274].alternate_names.push_back( PlayerName("verdi") );
	players[274].alternate_names.push_back( PlayerName("weak") );
	players[274].alternate_names.push_back( PlayerName("weakplayer") );
	players[21].alternate_names.push_back( PlayerName("seed") );
	players[21].alternate_names.push_back( PlayerName("vengeance") );
	players[368].alternate_names.push_back( PlayerName("Kamil Sudor") );
	players[117].alternate_names.push_back( PlayerName("ragnarok kr") );
	players[7813].alternate_names.push_back( PlayerName("Mamba") );
	players[7813].alternate_names.push_back( PlayerName("BlackMamba") );
	players[184].alternate_names.push_back( PlayerName("major juan") );
	players[8204].alternate_names.push_back( PlayerName("Phoenix T") );
	
	MakeNewb(NULL,"Chaos","ca",T,8484);
	MakeNewb(NULL,"Increase","ru",P).alternate_names.push_back(PlayerName("Încréasè"));
	players[5143].alternate_names.push_back(PlayerName("Winter"));
	MakeNewb(NULL, "Mellanmjolk", "se", Z).alternate_names.push_back(PlayerName("Mellanmjölk"));

	players[7778].alternate_names.push_back( PlayerName("pappijoe") );
	players[5823].alternate_names.push_back( PlayerName("spectre spirit") );
	players[7817].alternate_names.push_back( PlayerName("ssak ro") );
	players[5823].alternate_names.push_back( PlayerName("spirit ro") );
	players[5417].alternate_names.push_back( PlayerName("tropic se") );
	SearchPlayer(";believe;p;ca" DEBUGARGS).alternate_names.push_back( PlayerName("NayA") );
	SearchPlayer(";chaos;t;ca" DEBUGARGS).alternate_names.push_back( PlayerName("Jer99"));
	SearchPlayer(";chaos;t;ca" DEBUGARGS).alternate_names.push_back( PlayerName("Chaos"));
	players[422].alternate_names.push_back(PlayerName("jinair true"));
	players[1664].alternate_names.push_back(PlayerName("galaxy shine"));
	players[62].alternate_names.push_back(PlayerName("mvp shine"));
	players[213].alternate_names.push_back(PlayerName("ivd.apocalypse"));*/

	players[184].alternate_names.push_back(PlayerName("major juan"));
	players[274].alternate_names.push_back(PlayerName("verdi"));
	players[274].alternate_names.push_back(PlayerName("weak"));
	players[274].alternate_names.push_back(PlayerName("weakplayer"));
	players[21].alternate_names.push_back(PlayerName("seed"));
	players[21].alternate_names.push_back(PlayerName("vengeance"));
	players[310].alternate_names.push_back(PlayerName("top 310"));
	players[6300].alternate_names.push_back(PlayerName("zeal 6300"));
	players[7999].alternate_names.push_back(PlayerName("Hino"));
	players[164].alternate_names.push_back(PlayerName("WhiteRa"));
	players[2046].alternate_names.push_back(PlayerName("panic"));
	MakeNewb("nb", "Imre", "fr", T, 7978);
	MakeNewb(NULL, "Kenjo", "fr", T, 10257);
	MakeNewb(NULL, "Saber", "cn", P, 4183);

	MakeNewb("ecv", "fazz", "nl", Z);
	MakeNewb("ghost unit", "mrwhisper", "uk", Z);
	MakeNewb("kompis", "wawwsamet", "se", P);
	MakeNewb("luminosity", "dead", "us", R);
	MakeNewb("kjeks", "celadon", "no", T);
	MakeNewb("kjeks", "khaldrogo", "no", Z);
	MakeNewb(NULL, "methodv2", "cl", P);
	MakeNewb(NULL, "nightowl", "se", Z);
	/*MakeNewb(NULL, "BreakYa", "ch", Z);
	MakeNewb("nB", "GoSuMoneY", "be", P);
	MakeNewb("aW", "Lemiche", "fr", Z);
	MakeNewb("HoA", "WidowMaker", "fr", Z);
	MakeNewb("Invasion", "Grumpy", "fr", Z);
	MakeNewb("Leisure", "Mraptor", "fr", P);
	MakeNewb("MadmoiZerg", "Crunchy", "fr", Z);
	MakeNewb("orKs", "Sarah", "fr", Z);
	MakeNewb("Pentamed", "Kenjo", "fr", T);
	MakeNewb("RageQuit", "Odelay", "fr", T);
	MakeNewb("War Legend", "FrostMind", "fr", Z);
	MakeNewb("RageQuit", "Silent", "fr", P);
	MakeNewb(NULL, "Nepou", "fr", P);
	MakeNewb(NULL, "Konekosan", "fr", T);
	MakeNewb(NULL, "Spearline", "fr", T);
	MakeNewb(NULL, "Sapty", "fr", T);
	MakeNewb(NULL, "MacZerguey", "fr", Z);
	MakeNewb(NULL, "Poucky", "fr", Z);
	MakeNewb(NULL, "Godu", "fr", Z);
	MakeNewb(NULL, "Totoro", "fr", T);
	MakeNewb("tes", "VeniVidiVins", "it", Z);
	MakeNewb("MCp", "VacSaint", "fr", Z);
	MakeNewb("CpL", "Muyfresh", "fr", T);
	MakeNewb(NULL, "Invictus", "fr", Z);
	MakeNewb(NULL, "Symphoniaa", "fr", T);
	MakeNewb(NULL, "SbR", "fr", Z);
	MakeNewb(NULL, "Kafar", "fr", P);
	MakeNewb("TES", "Diord", "it", T);*/

	//int last_period=0;
	last_period = 0;
	for (size_t p = 0; p < players.size(); p++) {
		//players[p].player_id = p + 1;
		if (players[p].player_id == 0) continue;
		assert(players[p].player_id < MAX_PLAYER_ID-1);
		assert(players[p].player_id == p);
		//players[p].last_period=max(players[p].last_period,90);
		assert(players[p].last_period>0);
		last_period=max(last_period,players[p].last_period);
	}
	for (size_t t = 0; t < teams.size(); t++) {
		if (teams[t].team_id == 0) continue;
		assert(teams[t].team_id < MAX_TEAM_ID);
		assert(teams[t].team_id == t);
	}

	players[0].name="walkover";
	players[0].alternate_names.push_back( PlayerName("") );
	players[0].alternate_names.push_back( PlayerName("TBD") );
	players[0].alternate_names.push_back(PlayerName("BYE"));
	players[0].team_id=0;
	players[0].race=R;
	players[0].player_id=0;
	players[0].ratings[T]=players[0].ratings[Z]=players[0].ratings[P]=players[0].ratings[R]=0;

	good_players.clear();
	for(uint i=0;i<players.size();i++) {
		if(players[i].player_id==0) continue;
		if(players[i].ratings[R] < 1300) continue;
		if(players[i].last_period < last_period-5) continue;
		good_players.push_back(&players[i]);
	}
	good_koreans.clear();
	for (uint i = 0; i<players.size(); i++) {
		if (players[i].player_id == 0) continue;
		if (players[i].ratings[R] < 1300) continue;
		if (players[i].last_period < last_period - 5) continue;
		if (players[i].sCountry() != "kr") continue;
		good_koreans.push_back(&players[i]);
	}
	cerr << "good_players.size() == " << good_players.size() << ", good_koreans.size() == " << good_koreans.size() << "\n";

	cerr << "\n\nLoadPlayers complete\n\n\n--------------------------------\n\n\n";
}

void SavePlayers(MysqlCon &con)
{
	if(BDEBUG || BENCHMARK) {
		cerr << "Not saving players!\n";
		return;
	}
	cerr << "Saving players\n";
	con.StartTransaction();
	con.delete_query("delete from players where overall in(800,999,1199)" DEBUGARGS);

	string query = "insert into players (p_id,name,overall,vsP,vsT,vsZ,race,region,country,league,liquipedia,twitter,team_id,wcs_points,last_period) values ";
	for (auto &p : players) {
		if (p.player_id == 0) continue;
		string country(p.country, 2);
		query += "(" + ToString(p.player_id) + ",'" + con.escape(p.name.ToString(), 128) + "'," + ToString(p.ratings[R]) + "," + ToString(p.ratings[P]) + "," + ToString(p.ratings[T]) +
			"," + ToString(p.ratings[Z]) + ",'" + ToString((int)p.race) + "','','" + con.escape(country, 4) +
			"'," + ToString(p.league) + ",'liquipedia','twitter'," + ToString(p.team_id) + ",0,"+ ToString(p.last_period)+"),";
	}
	query = query.substr(0, query.length() - 1);
	query += " on duplicate key update name=values(name),overall=values(overall),vsP=values(vsP),vsT=values(vsT),vsZ=values(vsZ),race=values(race),region=values(region),country=values(country),league=values(league),liquipedia=values(liquipedia),twitter=values(twitter),team_id=values(team_id),wcs_points=values(wcs_points),league=values(league),last_period=values(last_period)";
	con.update(query.c_str() DEBUGARGS);

	query = "insert into teams (team_id,name,alternate_name,liquipedia) values ";
	for (auto &t : teams) {
		query += "(" + ToString(t.team_id) + ",'" + con.escape(t.name.ToString(), 1024) + "','','liquipedia'),";
	}
	query = query.substr(0, query.length() - 1);
	query += " on duplicate key update name=values(name),alternate_name=values(alternate_name),liquipedia=values(liquipedia)";
	con.update(query.c_str() DEBUGARGS);

	cerr << "\nSavePlayers complete\n\n\n";
}
