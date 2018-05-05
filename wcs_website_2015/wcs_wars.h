
uint PointsValue(uint min_points, uint median_points)
{
	wwglobals.GetData(DEBUGARG);
	uint wcsps = median_points + wwglobals.base_cost;
	//wcsps -= min_points/2;
	return wcsps;
}

uint PlayerValue(uint id)
{
	//gamesimheader.GetData(DEBUGARG);
	wwglobals.GetData(DEBUGARG);
	players.GetData(DEBUGARG);
	headbands.GetData(DEBUGARG);
	if(id>=players.players.size() || players.players[id].player_id!=id) return 0;
	auto &p = players.players[id];
	/*uint wcsps = p.medianpoints + wwglobals.base_cost;
	wcsps -= p.minpoints/2;*/
	uint wcsps = PointsValue(p.minpoints, p.medianpoints);
	
	if (p.headband == 1) wcsps += wwglobals.headband1cost;
	if (p.headband == 2) wcsps += wwglobals.headband2cost;
	if (p.headband == 3) wcsps += wwglobals.headband3cost;
	if (p.headband == 4) wcsps += wwglobals.headband4cost;
	return wcsps;
}

uint PlayerSellValue(uint id)
{
	uint wcsps = PlayerValue(id);
	wcsps = wcsps*wwglobals.sell_percent / 100;
	return wcsps;
}

class UserPlayer {
public:
	uint player_id, bought_for, sold_for, active, bought_on, sold_on, headband;
	UserPlayer() {
		player_id = 0;
		headband = 0;
	}
};

class UserTeam : public Module
{
public:
	vector<UserPlayer> team;
	uint teamvalue;
	User &user;
	bool for_update;

	UserTeam(User &tuser) : user(tuser)
	{
		//tuser.GetData(DEBUGARG);
		//user = tuser;
		teamvalue = 0;
		for_update=false;
	}

	void QueryForUpdate()
	{
		if(for_update && isloaded) return;
		for_update=true;
		isloaded=false;
		GetData(DEBUGARG);
	}
	
	virtual void RunQuery()
	{
		user.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
		string query="select player_id,bought_for,sold_for,active,unix_timestamp(bought_on),unix_timestamp(sold_on) from users_players where active=1 and user_id=" + ToString(user.user_id) + " order by player_id";
		if(for_update) query+=" for update";
		auto *res = mysqlcon.select(query.c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			UserPlayer up;
			up.player_id = row[0];
			up.bought_for = row[1];
			up.sold_for = row[2];
			up.active = row[3];
			up.bought_on = row[4];
			up.sold_on = row[5];
			if (up.player_id < players.players.size() && up.player_id == players.players[up.player_id].player_id) {
				up.headband = players.players[up.player_id].headband;
			}
			team.push_back(up);
		}
		isloaded = true;
		CalcTeamValue();
	}

	uint CalcTeamValue()
	{
		GetData(DEBUGARG);
		teamvalue = 0;
		for (auto &p : team) {
			teamvalue += PlayerValue(p.player_id);
		}
		return teamvalue;
	}

	string InnerHtml() {
		string teamjson;
		for (auto p : team) {
			teamjson += "{id:" + ToString(p.player_id) + ",headband:" + ToString(p.headband) + ",active:" + ToString(p.active) + "},";
		}
		if (teamjson.length()) teamjson = teamjson.substr(0, teamjson.length() - 1);
		return "<script>window.wcs.currentuser={id:" + ToString(user.user_id) + ",username:'" + mysqlcon.escape(user.username, 256) + "',wcspoints:" + ToString(user.wcspoints) + ",team:[" + teamjson + "]};</script>";
		//return "<script>window.wcs.currentuser={id:" + ToString(user_id) + ",username:'" + mysqlcon.MysqlEscape(username, 256) + "',wcspoints:" + ToString(wcspoints) + "};</script>";
	}
};
UserTeam current_user_team(current_user);

class UserAndTeam
{
public://dumbest class ever made T_T
	User u;
	UserTeam t;

	UserAndTeam() : t(u)
	{
	}

	UserAndTeam(User U) : u(U), t(u)
	{
	}

	UserAndTeam(const UserAndTeam &o) : u(o.u), t(u)
	{
		t.isloaded = o.t.isloaded;
		t.team = o.t.team;
		t.teamvalue = o.t.teamvalue;
	}

	UserAndTeam &operator=(const UserAndTeam &o)
	{
		u = o.u;
		t.isloaded = o.t.isloaded;
		t.team = o.t.team;
		t.teamvalue = o.t.teamvalue;
		return *this;
	}
};

class UserTeamSection : public Module
{
public:
	UserTeam &team;
	UserTeamSection(UserTeam &tteam) : team(tteam)
	{
	}

	virtual void RunQuery() {
		team.GetData(DEBUGARG);
		gamesimheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
	}
	
	virtual string InnerHtml() {
		string ret;
		string in;
		//uint samps = simheader.samples;
		auto &t = team.team;
		uint team_value = team.teamvalue;
		uint team_bought_for = 0;
		in += "<div class='wcsplayer-tiles'>";
		//string spids;
		for (uint i = 0; i<wwglobals.max_players || i<t.size(); i++) {
			if (i<t.size()) {
				auto &p = players.players[t[i].player_id];
				string phtml = players.phtml(t[i].player_id);
				uint worth = PlayerValue(p.player_id);
				team_bought_for += t[i].bought_for;
				uint sell_value = PlayerSellValue(p.player_id);
				//spids += ToString(p.player_id) + "+";
				if (team.user.user_id == current_user.user_id)
					in += "<span class='ptile'>" + phtml + "<br/>" + ToString(p.medianpoints) + " Median WCS Points<br/><button onclick=\"SellPlayer(" + ToString(p.player_id) + ")\" class='warn-button'>Sell this player<br/>" + ToString(sell_value) + " WCS Points</button><br/>Bought for " + ToString(t[i].bought_for) + "<br/>Now worth " + ToString(worth) + "</span>";
				else
					in += "<span class='ptile'>" + phtml + "<br/>" + ToString(p.medianpoints) + " Median WCS Points<br/>Bought for " + ToString(t[i].bought_for) + "<br/>Now worth " + ToString(worth) + "</span>";
			} else if (i<wwglobals.min_players) {
				in += "<span class='ptile ptileq'><div style='margin:auto;'>?</div></span>";
			} else {
				in += "<span class='ptile ptile-blank'>&nbsp;&nbsp;&nbsp;</span>";
			}
		}
		//if (spids.length()) spids = spids.substr(0, spids.length() - 1);
		in += "</div>";
		in = "<div>" + ToStringCommas(team.user.wcspoints) + " WCS Points remaining, and " + ToString(t.size()) + "/" + ToString(wwglobals.max_players) + " WCS Players, bought for " + ToStringCommas(team_bought_for) + " WCS Points, now worth " + ToStringCommas(team_value) + " WCS Points, overall worth " + ToStringCommas(team_value+team.user.wcspoints) + " WCS Points.</div>" + in;
		if (team.user.user_id == current_user.user_id) {
			in += "<rhr></rhr><mdiv>(You do not need to use all 5 player slots. At the end of each season a winner will be declared and all the teams will be reset.)</mdiv><div><button onclick='location.href=\"" + MakeURL("page=buyplayers") + "\"'><i class='fa fa-edit'></i>Buy and Sell WCS Players</button><button onclick=\"ResetPlayers()\" class='warn-button'><i class='fa fa-bomb'></i>Reset My WCS Players</button><button onclick='location.href=\"" + MakeURL("page=user&uid=" + ToString(team.user.user_id)) + "\"'><i class='fa fa-line-chart'></i>Team Stats</button></div>";
			ret += Section(string("Your Team"), in, "your-team");

			ret += "<script>if(!window.wcs.playersmed) { window.wcs.playersmed=[]; } window.wcs.playersmed.push(";
			auto &t = team.team;
			for (uint i = 0; i < t.size(); i++) {
				auto &p = players.players[t[i].player_id];
				ret += players.pMedJSON(p, 0);
			}
			if (ret.length() > 0) ret = ret.substr(0, ret.length() - 1);
			ret += ");</script>";
		} else {
			//in += "<rhr></rhr><div><button onclick='location.href=\"" + MakeURL("page=user&uid=" + ToString(team.user.user_id)) + "\"'><i class='fa fa-line-chart'></i>Team Stats</button></div>";
			ret += Section(team.user.namehtml() + "'s Team", in, "user-team");
		}
		return ret;
	}
};

const uint CURRENT_MATCH_VERSION = 1;
const uint base_ability_unit_type = 20000;
const uint base_unit_type = 30000;
const uint airport_type = base_unit_type + 1;
const uint generic_foreigner_type = base_unit_type + 2;
const uint generic_korean_type = base_unit_type + 3;
const uint internet_cafe_type = base_unit_type + 4;

class Unit
{
public:
	uint unit_id, user_id, unit_type, maxhp, hp, maxmana, mana, damage, xpos, ypos;
	uint speed, range;
	int armor, data1, data2, cooldown;
	bool touched;
	int isdef;

	Unit()
	{
		speed = range = 4;
		isdef = 0;
		touched = false;
		armor = 0;
		data1 = data2 = cooldown = 0;
		unit_id = 0;
	}

	string MysqlInsertValues(uint match_id)
	{
		string ret = "(" + ToString(match_id) + "," + ToString(user_id) + "," + ToString(unit_type) + "," + ToString(maxhp) + "," + ToString(hp) + "," + ToString(maxmana) + "," + ToString(mana) + "," + ToString(damage) + "," + ToString(xpos) + "," + ToString(ypos) + "," + ToString(data1) + "," + ToString(data2) + "," + ToString(cooldown) + "," + ToString(speed) + "," + ToString(range) + "," + ToString(armor) + "," + ToString(isdef) + ")";//(match_id,user_id,unit_type,maxhp,hp,maxmana,mana,damage,xpos,ypos,data1,data2,cooldown,speed,range,armor,isdef)
		return ret;
	}

	string name()
	{
		//return string("unknown unit type");
		if (unit_type<base_unit_type) {
			players.GetData(DEBUGARG);
			if (unit_type<players.players.size() && players.players[unit_type].player_id == unit_type) {
				return players.players[unit_type].name.ToString();
			}
		}
		if (unit_type == airport_type) return string("Airport");
		if (unit_type == generic_foreigner_type) return string("Generic Foreigner");
		if (unit_type == generic_korean_type) return string("Generic Korean");
		if (unit_type == internet_cafe_type) return string("Internet Cafe");
		return string("unknown unit type");
	}
	
	char race()
	{
		if (unit_type<base_unit_type) {
			players.GetData(DEBUGARG);
			if (unit_type<players.players.size() && players.players[unit_type].player_id == unit_type) {
				return players.players[unit_type].race;
			}
		}
		return R;
	}

	void init()
	{
		//here I will set values like range, speed, maxhp, maxmana? or this stuff should probably come from the database so it doesn't change mid-match?
		touched = false;
	}
};

class MapBase
{
public:
	byte terrain[128][128];
	uint width, height;
	string name;
	string format;

	virtual vector<Unit> InitMatchUnits(vector<UserAndTeam> users, vector<uint> user_teams) = 0;
	virtual string Json() {
		string ret = "{\"name\":\"" + mysqlcon.escape(name, 256) + "\",\"width\":" + ToString(width) + ",\"height\":" + ToString(height) + "}";
		return ret;
	}
	virtual void EvaluateLoseCondition(vector<uint> &user_ids, vector<uint> &user_teams, vector<Unit> &units)//what should this return? a vector of user_ids who have lost? the team id that lost?
	{
	}

	MapBase()
	{
		memset(terrain, 0, sizeof(terrain));
		height = 24;//16x16 smallest sane size? 64x64 biggest sane size?
		width = 32;
	}
};

namespace MapsV1 {
	class MapBaseV1 : public MapBase
	{
	public:
		virtual vector<Unit> InitMatchUnits(vector<UserAndTeam> users, vector<uint> user_teams)
		{
			wwglobals.GetData(DEBUGARG);
			players.GetData(DEBUGARG);
			vector<Unit> ret;

			Unit airport;
			airport.unit_type = airport_type;
			airport.speed = airport.range = airport.damage = 0;
			airport.maxhp = airport.hp = 10000;
			airport.armor = 500;
			airport.mana = airport.maxmana = 0;
			airport.isdef = 1;
			for (uint i = 0; i<users.size(); i++) {
				airport.user_id = users[i].u.user_id;
				ret.push_back(airport);
			}

			Unit icafe;
			icafe.unit_type=internet_cafe_type;
			icafe.speed= icafe.range=0;
			icafe.maxhp=icafe.hp=10000;
			icafe.armor=500;
			icafe.maxmana=icafe.mana=1000;
			icafe.isdef=1;
			icafe.user_id=0;
			ret.push_back(icafe);
			icafe.isdef=0;
			for(uint x=1;x<width;x+=10) {
				for(uint y=1;y<height;y+=10) {
					icafe.xpos=x;
					icafe.ypos=y;
					ret.push_back(icafe);
				}
			}

			Unit genforeigner;
			genforeigner.unit_type = generic_foreigner_type;
			genforeigner.speed = 5;
			genforeigner.range = 1;
			genforeigner.hp = genforeigner.maxhp = 4000;
			genforeigner.damage = 1000;
			genforeigner.mana = genforeigner.maxmana = 0;
			genforeigner.armor = 0;
			genforeigner.isdef = 1;
			for (uint i = 0; i<users.size(); i++) {
				genforeigner.user_id = users[i].u.user_id;
				ret.push_back(genforeigner);
			}

			Unit genkorean = genforeigner;
			genkorean.unit_type = generic_korean_type;
			genkorean.hp=genkorean.maxhp=5000;
			genkorean.damage = 1200;
			genkorean.range = 3;
			genkorean.armor = 100;
			for (uint i = 0; i<users.size(); i++) {
				genkorean.user_id = users[i].u.user_id;
				ret.push_back(genkorean);
			}

			for (uint i = 0; i<users.size(); i++) {
				auto &us = users[i];
				us.t.GetData(DEBUGARG);
				auto &u = us.u;
				uint uid = u.user_id;
				auto &t = us.t;
				for (uint j = 0; j<t.team.size(); j++) {
					auto &p = t.team[j];
					Unit u;
					u.user_id = uid;
					u.unit_type = p.player_id;
					u.damage = players.players[p.player_id].medianpoints + wwglobals.base_damage;
					u.maxhp = u.hp = 7000;
					u.maxmana = u.mana = 0;
					u.speed = 4;
					u.range = 7;
					//u.xpos=j+2;
					//u.ypos=i*4+2;
					u.isdef = 1;
					u.armor = 100;
					u.init();
					ret.push_back(u);
				}

				Unit tap = airport;
				tap.user_id = u.user_id;
				tap.xpos = i * 4 + 2;
				tap.ypos = 2;
				tap.isdef = 0;
				ret.push_back(tap);

				genforeigner.user_id = u.user_id;
				genforeigner.xpos = i * 4 + 2;
				genforeigner.ypos = 3;
				genforeigner.isdef = 0;
				ret.push_back(genforeigner);
				genforeigner.ypos = 4;
				ret.push_back(genforeigner);
			}
			for (auto &u : ret) u.touched = true;
			return ret;
		}

		string Json()
		{
			string ret = "{\"name\":\"" + mysqlcon.escape(name, 256) + "\",\"width\":" + ToString(width) + ",\"height\":" + ToString(height) + "}";
			return ret;//need to output the types of terrain with a name, description, and color, and also output the array of the map terrain
		}
	};

	class PlainsOfWar : public MapBaseV1
	{
	public:
	};
};
std::shared_ptr<MapBase> CreateMapObject(uint version, string format, string map_name)
{
	return std::make_shared<MapsV1::PlainsOfWar>(MapsV1::PlainsOfWar());
}

class MatchBase
{
public:
	uint match_id;
	uint version;
	string format;
	std::shared_ptr<MapBase> map;
	uint created;
	uint turn;
	uint turn_current_player;
	
	uint action_points;
	uint build_points;
	uint move_points;
	uint attack_points;
	uint special_points;
	uint reset_action_points;
	
	bool touched;
	bool for_update;
	vector<Unit> units;
	vector<Unit> unitdefs;
	vector<uint> user_ids;
	vector<uint> user_teams;
	vector<string> usernames;
	vector<int> users_money;
	vector<int> users_wonlost;
	int active;

	MatchBase(uint Version, string Format, string MapName) : format(Format)
	{
		for_update=false;
		match_id = 0;
		version = Version;
		map = CreateMapObject(version, format, MapName);
		touched = false;
		action_points=0;
		build_points=move_points=attack_points=special_points=0;
		reset_action_points=0x01010101;//1/1/1/1
		active=0;
	}

	virtual ~MatchBase()
	{
		string query;
		for (auto &u : units) {
			if (u.unit_id == 0 || u.touched) {
				touched = true;//maybe not neccessary to update the match row when a unit changes though...
			}
		}
		if (touched) {
			action_points = (special_points<<24) + (build_points<<16) + (move_points<<8) + (attack_points);
			query = "update matches set turn=" + ToString(turn) + ",active="+ToString(active)+",turn_current_player=" + ToString(turn_current_player) + ", action_points="+ToString(action_points)+" where match_id=" + ToString(match_id);
			mysqlcon.update(query.c_str() DEBUGARGS);

			query="";
			for(uint i=0;i<user_ids.size();i++) {
				query+="("+ToString(match_id)+","+ToString(user_ids[i])+","+ToString(users_money[i])+","+ToString(active)+",0,0,0,"+ToString(users_wonlost[i])+"),";
			}
			query = query.substr(0, query.length() - 1);
			query = "insert into match_players(match_id,user_id,money,active,last_mid_read,last_alerted_turn,team_id,wonlost) values"+query+" on duplicate key update money=values(money),wonlost=values(wonlost),active=values(active)";
			mysqlcon.update(query.c_str() DEBUGARGS);
		}

		for (auto &u : units) {
			if (u.unit_id != 0) continue;
			query = "insert into match_units(match_id,user_id,unit_type,maxhp,hp,maxmana,mana,damage,xpos,ypos,data1,data2,cooldown,speed,attack_range,armor,isdef) values";
			query += "(" + ToString(match_id) + "," + ToString(u.user_id) + "," + ToString(u.unit_type) + "," + ToString(u.maxhp) + "," + ToString(u.hp) + "," + ToString(u.maxmana) + "," + ToString(u.mana) + "," + ToString(u.damage) + "," + ToString(u.xpos) + "," + ToString(u.ypos) + "," + ToString(u.data1) + "," + ToString(u.data2) + "," + ToString(u.cooldown) + "," + ToString(u.speed) + "," + ToString(u.range) + "," + ToString(u.armor) + ",0)";
			u.unit_id = (uint)mysqlcon.insert(query.c_str() DEBUGARGS);
			u.touched = false;
		}
		for (auto &u : units) {
			if (u.unit_id == 0) {
				DEBUGOUT(FATAL, u.unit_id, "this should never happen!");
			}
		}
		query = "";
		for (auto &u : units) {
			if (u.touched == false) continue;
			query += "(" + ToString(match_id) + "," + ToString(u.user_id) + "," + ToString(u.unit_type) + "," + ToString(u.maxhp) + "," + ToString(u.hp) + "," + ToString(u.maxmana) + "," + ToString(u.mana) + "," + ToString(u.damage) + "," + ToString(u.xpos) + "," + ToString(u.ypos) + "," + ToString(u.data1) + "," + ToString(u.data2) + "," + ToString(u.cooldown) + "," + ToString(u.unit_id) + "," + ToString(u.speed) + "," + ToString(u.range) + "," + ToString(u.armor) + ",0),";
		}
		if (query.length()) {
			query = query.substr(0, query.length() - 1);
			//do an update, inserts will need to be done separately, might still need to do this as an insert/update? lol
			query = "insert into match_units(match_id,user_id,unit_type,maxhp,hp,maxmana,mana,damage,xpos,ypos,data1,data2,cooldown,unit_id,speed,attack_range,armor,isdef) values" + query + " on duplicate key update user_id=values(user_id),unit_type=values(unit_type),hp=values(hp),mana=values(mana),damage=values(damage),xpos=values(xpos),ypos=values(ypos),data1=values(data1),data2=values(data2),cooldown=values(cooldown),armor=values(armor)";
			mysqlcon.update(query.c_str() DEBUGARGS);
		}
	}

	virtual void QueryUnits()
	{
		string query="select match_id,user_id,unit_type,hp,mana,damage,xpos,ypos,data1,data2,cooldown,unit_id,maxhp,maxmana,speed,attack_range,armor,isdef from match_units where match_id=" + ToString(match_id) + " and hp>0";
		if(for_update) query+=" for update";
		auto res = mysqlcon.select(query.c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			Unit u;
			u.user_id = row[1];
			u.unit_type = row[2];
			u.hp = row[3];
			u.mana = row[4];
			u.damage = row[5];
			u.xpos = row[6];
			u.ypos = row[7];
			u.data1 = row[8];
			u.data2 = row[9];
			u.cooldown = row[10];
			u.unit_id = row[11];
			u.maxhp = row[12];
			u.maxmana = row[13];
			u.speed = row[14];
			u.range = row[15];
			u.armor = row[16];
			int isdef = row[17];
			u.isdef=isdef;
			u.init();
			if (isdef) {
				unitdefs.push_back(u);
			} else {
				units.push_back(u);
			}
		}
		map->EvaluateLoseCondition(user_ids, user_teams, units);
	}

	virtual void ReadInitQuery(uint MatchId, int Active, vector<uint> &UserIds, vector<uint> &UserTeams, vector<string> &Usernames, vector<int> &UsersMoney, vector<int> &UsersWonLost, uint Turn, uint TurnCurrentPlayer, uint ActionPoints, bool For_Update DEBUGARGSDEF)
	{
		active=Active;
		match_id = MatchId;
		user_ids = UserIds;
		user_teams = UserTeams;
		usernames = Usernames;
		users_money = UsersMoney;
		users_wonlost = UsersWonLost;
		turn = Turn;
		turn_current_player = TurnCurrentPlayer;
		action_points = ActionPoints;
		for_update=For_Update;
		QueryUnits();
		
		if(action_points==0) {
			if(turn>1) DEBUGOUT(ERR, action_points, "should only happen on creation");
			action_points=reset_action_points;
		}
		special_points = action_points>>24;
		build_points = (action_points>>16) & 0xFF;
		move_points	 = (action_points>>8) & 0xFF;
		attack_points = (action_points) & 0xFF;
		assert(action_points == (special_points<<24) + (build_points<<16) + (move_points<<8) + (attack_points));
	}
	
	bool UnitIsVisible(Unit &u)
	{//need to take the current user into account and calculate visibility...
		return true;
	}
	
	bool TileIsVisible(uint x, uint y)
	{
		return true;
	}

	bool HaveMoney(uint user_id, int cost)
	{
		uint slot=0;
		for(;slot<user_ids.size();slot++) {
			if(user_ids[slot]==user_id) break;
		}
		if(slot>=user_ids.size()) {
			DEBUGOUTSTACK(ERR, user_id, "user_id not found?");
			return false;
		}
		if(cost>users_money[slot]) return false;
		return true;
	}

	bool ChargeMoney(uint user_id, int cost)
	{
		uint slot=0;
		for(;slot<user_ids.size();slot++) {
			if(user_ids[slot]==user_id) break;
		}
		if(slot>=user_ids.size()) {
			DEBUGOUTSTACK(ERR, user_id, "user_id not found?");
			return false;
		}
		if(cost>users_money[slot]) return false;
		users_money[slot]-=cost;
		touched=true;
		return true;
	}
	
	bool TickTurn(uint cost_special, uint cost_move, uint cost_attack, uint cost_build DEBUGARGSDEF)
	{
		if(cost_special>special_points || cost_move>move_points || cost_attack>attack_points || cost_build>build_points) {
			DEBUGOUTSTACK(ERR, action_points, "not enough action points, special=="<<special_points<<", move=="<<move_points<<", attack=="<<attack_points<<", build=="<<build_points);
			return false;
		}

		special_points-=cost_special;
		move_points-=cost_move;
		attack_points-=cost_attack;
		build_points-=cost_build;
		touched=true;

		uint buildings_teams[16];
		for(uint i=0;i<16;i++) buildings_teams[i]=0;
		vector<uint> buildings = user_ids;
		for(auto &b : buildings) b=0;
		for(auto &u : units) {
			if(u.unit_type!=airport_type) continue;
			for(uint i=0; i<user_ids.size(); i++) {
				if(u.user_id==user_ids[i]) {
					buildings[i]++;
					buildings_teams[ user_teams[i] ]++;
				}
			}
		}
		for(uint i=0; i<buildings.size();i++) {
			if(buildings[i]==0 && users_wonlost[i]==0) {
				//you lose faggarotto, maybe check teammates
				users_wonlost[i]=-1;
				touched=true;
			}
		}
		for(uint i=0;i<users_wonlost.size();i++) {
			if(users_wonlost[i]!=0) continue;

			bool won=true;
			for(uint b=0;b<users_wonlost.size();b++) {
				if(b==i) continue;
				if(user_teams[b] != user_teams[i] && users_wonlost[b]>0) {
					won=false;
					break;
				}
			}
			if(won) {
				users_wonlost[i]=1;
				touched=true;
			}
		}
		uint active_players=0;
		for(auto wonlost : users_wonlost) {
			if(wonlost==0) active_players++;
		}
		if(active_players==0) {
			active=0;
			touched=true;
			//return true;//continue on, SC1 and 2 provide this option so why not
		}
		if(active_players==1) DEBUGOUT(ERR, active_players, "wut");

		if(special_points+move_points+attack_points+build_points>0) return true;

		special_points=1;
		move_points=1;
		attack_points=1;
		build_points=1;
		
		//check win/lose conditions first...
		for(uint i=0;i<user_ids.size();i++) {
			uint p=user_ids[i];
			if(p==turn_current_player) {
				i++;
				for(;users_wonlost[i%users_wonlost.size()]<0;i++);
				turn_current_player=user_ids[i%user_ids.size()];
				if(i>=user_ids.size()-1) {
					turn++;
					TickUnits(DEBUGARG);
				}
				touched=true;
				return true;
			}
		}
		return true;
	}
	
	void TickUnits(DEBUGARGDEF)
	{
		for(uint u=0;u<units.size();u++) {
			if(units[u].unit_type==airport_type) {
				TickAirport(u DEBUGARGS);
			} else if(units[u].cooldown) {
				units[u].cooldown--;
				units[u].touched=true;
			}
		}
	}
	
	std::pair<uint,uint> GetBuildSpot(Unit &u DEBUGARGSDEF)
	{
		std::pair<uint,uint> ret;
		uint x=u.xpos;
		uint y=u.ypos;
		//uint width=map.width;
		//uint height=map.height;
		y++;
		ret.first=x;
		ret.second=y;
		return ret;
	}
	
	void TickAirport(uint uid DEBUGARGSDEF)
	{
		auto &u=units[uid];
		if(u.data1 && u.cooldown) {
			u.cooldown--;
			u.touched=true;
			if(u.cooldown==0) {
				std::pair<uint,uint> pos = GetBuildSpot(u DEBUGARGS);
				Unit n;
				n.unit_type=0;
				for(auto &d : unitdefs) {
					if(d.unit_type==(uint)u.data1 && d.user_id==u.user_id) {
						n=d;
						break;
					}
				}
				if(n.unit_type==0) {
					u.data1=0;
					return;
				}
				n.unit_id=0;
				n.xpos=pos.first;
				n.ypos=pos.second;
				n.cooldown=0;
				n.isdef=0;
				//n.touched=true;//the 0 unit_id is enough...
				n.init();
				u.data1=0;
				units.push_back(n);
				DEBUGOUTSTACK(ERR, n.name(), "created unit");
			}
		} else if(u.data1 && u.cooldown==0) {
			DEBUGOUTSTACK(ERR, u.data1, "cooldown==0");
			u.data1=0;
			u.touched=true;
		} else if(u.data1==0 && u.cooldown) {
			DEBUGOUTSTACK(ERR, u.cooldown, "data1==0");
			u.cooldown=0;
			u.touched=true;
		}
	}
	
	string UnitsJson()
	{
		//players.GetData(DEBUGARG);
		//return string("[]");
		string junits="[";
		for(uint i=0;i<units.size();i++) {
			auto &u = units[i];
			if(u.hp<=0) continue;
			if(UnitIsVisible(u)) {
				junits+="{\"unit_id\":"+ToString(u.unit_id)+",\"user_id\":"+ToString(u.user_id)+",\"unit_type\":"+ToString(u.unit_type)+",\"hp\":"+ToString(u.hp)+",\"mana\":"+ToString(u.mana)+",\"damage\":"+ToString(u.damage)+",\"x\":"+ToString(u.xpos)+",\"y\":"+ToString(u.ypos)+",\"d1\":"+ToString(u.data1)+",\"d2\":"+ToString(u.data2)+",\"cooldown\":"+ToString(u.cooldown)+",\"speed\":"+ToString(u.speed)+",\"range\":"+ToString(u.range)+",\"maxhp\":"+ToString(u.maxhp)+",\"maxmana\":"+ToString(u.maxmana)+"},";
			}
		}
		if(junits.length()>1) junits=junits.substr(0,junits.length()-1);
		junits+="]";
		return junits;
	}
	
	string UnitTypesJson()
	{
		//return string("[]");
		current_user.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		//return string("[]");
		string junits="[";
		string airport_abilities;
		for(uint i=0;i<unitdefs.size();i++) {
			auto &u = unitdefs[i];
			if(u.user_id!=current_user.user_id) continue;
			if(u.unit_type==airport_type) continue;//this is done after with all the abilities....definitely need a better way to handle abilities
			string name=u.name();
			uint cost=u.damage;
			airport_abilities+="{\"id\":"+ToString(u.unit_type)+",\"type\":1,\"name\":\"Build "+mysqlcon.escape(name,128)+"\",\"cost\":"+ToString(cost)+",\"cast_time\":4,\"description\":\"Build this unit\"},";
		}
		if(airport_abilities.length()) airport_abilities=airport_abilities.substr(0,airport_abilities.length()-1);
		string protoss_abilities =
		"{\"id\":1,\"type\":2,\"name\":\"Stream\",\"cost\":-2000, \"cast_time\":3,\"description\":\"Stream on twitch at an Internet Cafe to make money\"},"
		"{\"id\":2,\"type\":3,\"name\":\"Psi Storm\",\"cost\":0, \"mana_cost\":75, \"cast_time\":1,\"description\":\"Cast Psionic Storm.\"}";
		
		string terran_abilities =
		"{\"id\":1,\"type\":2,\"name\":\"Stream\",\"cost\":-2000, \"cast_time\":3,\"description\":\"Stream on twitch at an Internet Cafe to make money\"},"
		"{\"id\":3,\"type\":3,\"name\":\"Nuke\",\"cost\":500, \"mana_cost\":50, \"cast_time\":3,\"description\":\"Launch a nuclear missile.\"}";

		string zerg_abilities =
		"{\"id\":1,\"type\":2,\"name\":\"Stream\",\"cost\":-2000, \"cast_time\":3,\"description\":\"Stream on twitch at an Internet Cafe to make money\"},"
		"{\"id\":4,\"type\":2,\"name\":\"Abduct\",\"cost\":0, \"mana_cost\":50, \"cast_time\":0,\"description\":\"Abduct a unit.\"}";
		
		//return string("[]");
		for(uint i=0;i<unitdefs.size();i++) {
			auto &u = unitdefs[i];
			//if(u.user_id!=current_user.user_id) continue;
			string abilities;
			if(u.user_id==current_user.user_id && u.unit_type==airport_type) abilities=airport_abilities;
			else if(u.user_id==current_user.user_id && u.unit_type<base_unit_type) {
				auto race=R;
				if(players.players.size()>u.unit_type && players.players[u.unit_type].player_id==u.unit_type) {
					race=players.players[u.unit_type].race;
				}
				if(race==T) abilities=terran_abilities;
				if(race==P) abilities=protoss_abilities;
				if(race==Z) abilities=zerg_abilities;
			}
			else if(u.user_id==current_user.user_id) abilities="{\"id\":1,\"type\":2,\"name\":\"Stream\",\"cost\":-2000, \"cast_time\":3,\"description\":\"Stream on twitch at an Internet Cafe to make money\"}";
			
			junits+="{\"unit_id\":"+ToString(u.unit_id)+",\"user_id\":"+ToString(u.user_id)+",\"unit_type\":"+ToString(u.unit_type)+",\"hp\":"+ToString(u.hp)+",\"mana\":"+ToString(u.mana)+",\"damage\":"+ToString(u.damage)+",\"x\":"+ToString(u.xpos)+",\"y\":"+ToString(u.ypos)+",\"d1\":"+ToString(u.data1)+",\"d2\":"+ToString(u.data2)+",\"cooldown\":"+ToString(u.cooldown)+",\"name\":\""+mysqlcon.escape(u.name(),256)+"\",\"speed\":"+ToString(u.speed)+",\"range\":"+ToString(u.range)+",\"maxhp\":"+ToString(u.maxhp)+",\"maxmana\":"+ToString(u.maxmana)+",\"abilities\":["+abilities+"]},";
		}
		if(junits.length()>1) junits=junits.substr(0,junits.length()-1);
		junits+="]";
		return junits;
	}
	
	string UsersJson()
	{
		string users="[";
		for(uint i=0;i<user_ids.size();i++) {
			users+="{\"user_id\":"+ToString(user_ids[i])+",\"username\":\""+mysqlcon.escape(usernames[i],256)+"\",\"team\":"+ToString(user_teams[i])+"},";
		}
		if(users.length()) users=users.substr(0,users.length()-1);
		users+="]";
		return users;
	}
	
	virtual string Html()
	{
		//return string("{}");
		string ret;
		uint taction_points=action_points;
		uint tspecial_points=special_points;
		uint tmove_points=move_points;
		uint tattack_points=attack_points;
		uint tbuild_points=build_points;
		if(turn_current_player!=current_user.user_id) {
			taction_points=0;
			tspecial_points=0;
			tmove_points=0;
			tattack_points=0;
			tbuild_points=0;
		}
		uint money=0;
		int wonlost=0;
		for(uint i=0;i<user_ids.size();i++) {
			if(user_ids[i]==current_user.user_id) {
				money=users_money[i];
				wonlost=users_wonlost[i];
			}
		}

		ret+="{\"match_id\":"+ToString(match_id)+",\"format\":\""+mysqlcon.escape(format,32)+"\",\"version\":"+ToString(version)+",\"turn\":"+ToString(turn)+",\"turn_current_player\":"+ToString(turn_current_player)+",";
		ret+="\"action_points\":"+ToString(taction_points)+",";
		ret+="\"special_points\":"+ToString(tspecial_points)+",\"move_points\":"+ToString(tmove_points)+",\"attack_points\":"+ToString(tattack_points)+",\"build_points\":"+ToString(tbuild_points)+",";
		ret+="\"money\":"+ToString(money)+",";
		if(wonlost) ret+="\"wonlost\":"+ToString(wonlost)+",";
		ret+="\"usersa\":"+UsersJson()+",";
		ret+="\"unitsa\":"+UnitsJson()+",";
		ret+="\"unittypesa\":"+UnitTypesJson()+",";
		ret+="\"map\":"+map->Json();
		ret+="}";
		return ret;
	}
	
	string GetChat(uint lasttime)
	{
		string query;
		if(lasttime>0) {
			query = "select message_id, user_id, unix_timestamp(created), message, username, unix_timestamp(now()) from match_messages join users using(user_id) where created>now()-interval 1 day and created>=from_unixtime("+ToString(lasttime)+")-interval 15 second and match_id="+ToString(match_id)+" order by created desc limit 50";
		} else {
			query = "select message_id, user_id, unix_timestamp(created), message, username, unix_timestamp(now()) from match_messages join users using(user_id) where created>now()-interval 1 day and match_id="+ToString(match_id)+" order by created desc limit 50";
		}
		auto *res = mysqlcon.select(query.c_str() DEBUGARGS);
		auto rows=mysql_num_rows(res);
		uint now=0;
		string messages;
		for(uint i=0;i<rows;i++) {
			auto r=mysqlcon.fetch_row(res);
			uint mid=r[0];
			uint uid=r[1];
			uint c=r[2];
			string t=r[3].cstr();
			string n=r[4].cstr();
			now=r[5];
			messages+="{\"mid\":"+ToString(mid)+",\"uid\":"+ToString(uid)+",\"c\":"+ToString(c)+",\"t\":\""+jsonescape(t,4096)+"\",\"n\":\""+jsonescape(n,1024)+"\"},";
		}
		if(messages.length()) messages=messages.substr(0, messages.length()-1);
		string ret="{\"now\":"+ToString(now)+",\"messages\":["+messages+"]}";
		return ret;
	}

	void AddChatMessage(uint user_id, string message)
	{
		string query="insert into match_messages set user_id="+ToString(user_id)+", match_id="+ToString(match_id)+", created=now(), message='"+mysqlcon.escape(message, 1024)+"'";
		mysqlcon.insert(query.c_str() DEBUGARGS);
	}

	string PostChatMessage(string message)
	{
		current_user.GetData(DEBUGARG);
		message=URLDecode(message);
		if(message.length()>1024) return string("{\"success\":0,\"error\":\"Message is too long\",\"chat\":"+GetChat((uint)vars["lasttime"])+"}");
		AddChatMessage(current_user.user_id, message);
		return string("{\"success\":1,\"chat\":"+GetChat((uint)vars["lasttime"])+"}");
	}

	virtual string UpdatedGameState()
	{
		//current_user.GetData(DEBUGARG);
		string ret;
		uint taction_points=action_points;
		uint tspecial_points=special_points;
		uint tmove_points=move_points;
		uint tattack_points=attack_points;
		uint tbuild_points=build_points;
		if(turn_current_player!=current_user.user_id) {
			taction_points=0;
			tspecial_points=0;
			tmove_points=0;
			tattack_points=0;
			tbuild_points=0;
		}
		uint money=0;
		int wonlost=0;
		for(uint i=0;i<user_ids.size();i++) {
			if(user_ids[i]==current_user.user_id) {
				money=users_money[i];
				wonlost=users_wonlost[i];
			}
		}

		ret+="{\"match_id\":"+ToString(match_id)+",\"format\":\""+mysqlcon.escape(format,32)+"\",\"version\":"+ToString(version)+",\"turn\":"+ToString(turn)+",\"turn_current_player\":"+ToString(turn_current_player)+",";
		ret+="\"action_points\":"+ToString(taction_points)+",";
		ret+="\"special_points\":"+ToString(tspecial_points)+",\"move_points\":"+ToString(tmove_points)+",\"attack_points\":"+ToString(tattack_points)+",\"build_points\":"+ToString(tbuild_points)+",";
		ret+="\"money\":"+ToString(money)+",";
		if(wonlost) ret+="\"wonlost\":"+ToString(wonlost)+",";

		ret+="\"usersa\":"+UsersJson()+",";
		ret+="\"unitsa\":"+UnitsJson()+",";

		ret+="\"chat\":"+GetChat((uint)vars["lasttime"]);
		ret+="}";
		return ret;
	}
	
	string IssueMoveOrder(Unit &u, uint target_x, uint target_y DEBUGARGSDEF)
	{
		//wwglobals.GetData(DEBUGARG);
		current_user.GetData(DEBUGARG);
		
		if(TileIsVisible(target_x,target_y)==false) {
			return string("{\"success\":0,\"error\":\"you cannot see the destination\"}");
		}
		
		for(uint i=0;i<units.size();i++) {
			auto &u = units[i];
			if(u.xpos==target_x && u.ypos==target_y) {
				return string("{\"success\":0,\"error\":\"this tile is occupied\"}");
			}
		}
		uint myx=u.xpos;
		uint myy=u.ypos;
		
		int distx = abs((int)myx-(int)target_x);
		int disty = abs((int)myy-(int)target_y);
		double ddist = (double)(distx*distx + disty*disty);
		uint dist = (uint)sqrt(ddist);
		if(dist>u.speed) {
			return string("{\"success\":0,\"error\":\"target is too far, with a distance of "+ToString(dist)+"\"}");
		}
		if(TickTurn(0,1,0,0 DEBUGARGS)==false) {
			return string("{\"success\":0,\"error\":\"You do not have enough action points to do this in this turn\"}");
		}
		u.xpos=target_x;
		u.ypos=target_y;
		u.touched=true;
		
		return string("{\"success\":1}");
	}
	
	int CalcDamage(Unit &u, Unit &t)
	{
		int dam=u.damage-t.armor;
		if(dam>(int)t.hp) dam=t.hp;
		return dam;
	}
	
	string IssueAttackOrder(Unit &u, Unit &t DEBUGARGSDEF)
	{
		if(u.damage==0) {
			return string("{\"success\":0,\"error\":\"This unit cannot attack\"}");
		}
		//wwglobals.GetData(DEBUGARG);
		current_user.GetData(DEBUGARG);
		
		uint target_x=t.xpos;
		uint target_y=t.ypos;
		uint myx=u.xpos;
		uint myy=u.ypos;
		
		int distx = abs((int)myx-(int)target_x);
		int disty = abs((int)myy-(int)target_y);
		double ddist = (double)(distx*distx + disty*disty);
		uint dist = (uint)sqrt(ddist);
		if(dist>u.range) {
			return string("{\"success\":0,\"error\":\"target is too far, with a distance of "+ToString(dist)+"\"}");
		}
		if(TickTurn(0,0,1,0 DEBUGARGS)==false) {
			return string("{\"success\":0,\"error\":\"You do not have enough action points to do this in this turn\"}");
		}
		u.cooldown=2;
		u.touched=true;
		int damage=CalcDamage(u,t);
		t.hp-=damage;
		t.touched=true;
		if(t.hp<=0) {
			string attackerusername;
			string loserusername;
			for(uint i=0;i<user_ids.size();i++) {
				if(u.user_id==user_ids[i]) attackerusername=usernames[i];
				if(t.user_id==user_ids[i]) loserusername=usernames[i];
			}
			AddChatMessage(0, attackerusername+"'s "+u.name()+" has slain "+loserusername+"'s "+t.name());
		}
		
		return string("{\"success\":1}");
	}
	
	string IssueAirportAbility(Unit &u, int ability_id DEBUGARGSDEF)
	{
		Unit udef;
		uint unit_type=0;
		for(auto &def : unitdefs) {
			if(def.unit_type==(uint)ability_id && def.user_id==current_user.user_id) {
				//make this unit!
				unit_type=def.unit_type;
				udef=def;
				break;
			}
		}
		if(unit_type==0) {
			return string("{\"success\":0,\"error\":\"unit definition not found\"}");
		}
		bool alreadyhave=false;
		if(unit_type<MAX_PLAYER_ID) {
			for(auto &u : units) {
				if(u.user_id==current_user.user_id && u.hp>0 && u.unit_type==unit_type) alreadyhave=true;
			}
		}
		if(alreadyhave) return string("{\"success\":0,\"error\":\"You already have one of those\"}");

		uint cost=udef.damage;
		if(HaveMoney(u.user_id, cost)==false) {
			return string("{\"success\":0,\"error\":\"You do not have enough money\"}");
		}
		if(TickTurn(0,0,0,1 DEBUGARGS)==false) {
			return string("{\"success\":0,\"error\":\"You do not have enough action points to do this in this turn\"}");
		}
		if(ChargeMoney(u.user_id, cost)==false) {
			DEBUGOUTSTACK(ERR, "uh oh", "");
			return string("{\"success\":0,\"error\":\"You do not have enough money\"}");
		}
		//now find a spot for it to appear, or do this after the build time....how to do build time? data1==unit_type and data2==turns left? could work
		u.data1=unit_type;
		uint build_time=4;
		u.cooldown=build_time;
		u.touched=true;
		
		return string("{\"success\":1}");
	}
	
	string IssueAbilityOrder(Unit &u, int ability_id DEBUGARGSDEF)
	{
		current_user.GetData(DEBUGARG);
		
		//are abilities map specific? not really, maybe just some variables for them which can be stored in the unit isdef...ex in SC2 psi storm is always psi storm you can just change the size,duration,damage
		if(u.unit_type==airport_type) {
			return IssueAirportAbility(u,ability_id DEBUGARGS);
		}/* else if(ability_id==1) {//stream
			//first check nearby icafe and make sure it has enough mana/cooldown?
			uint manacost=1;
			for(auto &i : units) {
				if(i.unit_type!=internet_cafe_type) continue;
			}
			if(TickTurn(1,0,0,0 DEBUGARGS)==false) {
				return string("{\"success\":0,\"error\":\"You do not have enough action points to do this in this turn\"}");
			}
			u.cooldown=3;
			u.touched=true;
			ChargeMoney(u.user_id, -(int)u.damage);
			return string("{\"success\":1}");
		}*/
		return string("{\"success\":0,\"error\":\"this unit has no abilities\"}");
	}
	
	string IssueAbilityTargetPosOrder(Unit &u, int ability_id, uint target_x, uint target_y DEBUGARGSDEF)
	{
		if(ability_id==2) {
			//create the units for each tile under the storm
			if(u.race()!=P) {
				return string("{\"success\":0,\"error\":\"only Protoss units can use Psi Storm\"}");
			}
			return string("{\"success\":0,\"error\":\"Psi Storm not yet impletemented\"}");
		} else if(ability_id==3) {
			//create the unit for the laser dot
			if(u.race()!=T) {
				return string("{\"success\":0,\"error\":\"only Terran units can use Nuke\"}");
			}
			return string("{\"success\":0,\"error\":\"Nuke not yet impletemented\"}");
		}
		return string("{\"success\":0,\"error\":\"not yet impletemented\"}");
	}
	
	string IssueAbilityTargetOrder(Unit &u, int ability_id, Unit &t DEBUGARGSDEF)
	{
		if(u.unit_type==airport_type) {
			return string("{\"success\":0,\"error\":\"this unit has no targetted abilities\"}");
		}
		if(ability_id==1) {
			if(t.unit_type!=internet_cafe_type) return string("{\"success\":0,\"error\":\"You must target an Internet Cafe\"}");
			uint manacost=1;
			uint range = 1;
			uint target_x = t.xpos;
			uint target_y = t.ypos;
			uint myx = u.xpos;
			uint myy = u.ypos;

			int distx = abs((int)myx - (int)target_x);
			int disty = abs((int)myy - (int)target_y);
			double ddist = (double)(distx*distx + disty*disty);
			uint dist = (uint)sqrt(ddist);
			if (dist > range) return string("{\"success\":0,\"error\":\"target is too far, with a distance of " + ToString(dist) + "\"}");
			if(t.mana<manacost) return string("{\"success\":0,\"error\":\"Internet Cafe does not have enough bandwidth(mana)\"}");
			if(TickTurn(1,0,0,0 DEBUGARGS)==false) return string("{\"success\":0,\"error\":\"You do not have enough action points to do this in this turn\"}");
			u.cooldown=3;
			u.touched=true;
			t.mana-=manacost;
			t.touched=true;
			ChargeMoney(u.user_id, -(int)u.damage);
			return string("{\"success\":1}");
		} else if(ability_id==4) {
			if(u.race()!=Z) {
				return string("{\"success\":0,\"error\":\"only Zerg units can use Abduct\"}");
			}
			return string("{\"success\":0,\"error\":\"Abduct not yet impletemented\"}");
		}
		return string("{\"success\":0,\"error\":\"not yet impletemented\"}");
	}

	virtual string IssueOrder(string order, uint myunit_id DEBUGARGSDEF)
    {
		//return string("{\"success\":0,\"error\":\"not implemented yet\"}");
        string ret;
        string resp;
        string gamestate;
		current_user.GetData(DEBUGARG);
        //Match match((uint)Variables["matchid"]);
		//match.GetData(DEBUGARG);
		if(current_user.user_id!=turn_current_player) {
			return string("{\"success\":0,\"error\":\"it's not your turn\"}");
		}
		
		/*if(Variables["target_x"] && Variables["target_y"]) {
		 if(match.TileIsVisible((uint)Variables["]target_x"],(uint)Variables["target_y"])==false) {
		 return string("{\"success\":\"0\",\"error\":\"you cannot see the destination\"}");
		 }
		 }*///this check here would ruin a scan-like ability, maybe this should just be done inside the move function and w/e else needs it
		
		size_t target_slot=units.size();
		size_t my_slot=units.size();
		if(vars["target_id"]) {
			uint target_id=vars["target_id"];
			for(uint i=0;i<units.size();i++) {
                auto &u = units[i];
                if(u.unit_id==target_id) {
                    bool target_is_visible=UnitIsVisible(u);
                    if(target_is_visible==false) {
                        return string("{\"success\":0,\"error\":\"you cannot see your target\"}");
                    }
                    target_slot=i;
                } else if(u.unit_id==myunit_id) {
                    if(u.user_id!=current_user.user_id) {
                        return string("{\"success\":0,\"error\":\"this unit does not belong to you\"}");
                    }
                    my_slot=i;
                }
            }
			if(target_slot>=units.size()) {
                return string("{\"success\":0,\"error\":\"could not find your target\"}");
            }
		} else {
			for(uint i=0;i<units.size();i++) {
				auto &u = units[i];
				if(u.unit_id==myunit_id) {
					if(u.user_id!=current_user.user_id) {
						return string("{\"success\":0,\"error\":\"this unit does not belong to you\"}");
					}
					my_slot=i;
					break;
				}
			}
			target_slot=0;
		}
		if(my_slot>=units.size()) {
			return string("{\"success\":0,\"error\":\"could not find your unit\"}");
		}
		Unit &u=units[my_slot];
		Unit &t=units[target_slot];
		
		if(order=="end_turn") {
			TickTurn(special_points, move_points, attack_points, build_points DEBUGARGS);
			resp="{\"success\":1}";
		} else if(u.cooldown) {
			resp="{\"success\":0,\"error\":\"this unit is on cooldown for "+ToString(u.cooldown)+" more turns\"}";
		} else if(order=="move") {
            resp=IssueMoveOrder(u, (uint)vars["target_x"], (uint)vars["target_y"] DEBUGARGS);
        } else if(order=="attack") {
            resp=IssueAttackOrder(u, t DEBUGARGS);
		} else if(order=="ability") {
			resp=IssueAbilityOrder(u, (int)vars["ability_id"] DEBUGARGS);
		} else if(order=="ability_target_pos") {
			resp=IssueAbilityTargetPosOrder(u, (int)vars["ability_id"], (uint)vars["target_x"], (uint)vars["target_y"] DEBUGARGS);
		} else if(order=="ability_target") {
			resp=IssueAbilityTargetOrder(u, (int)vars["ability_id"], t DEBUGARGS);
        } else {
            return string("{\"success\":0,\"error\":\"Unknown order type\"}");
        }
        gamestate=UpdatedGameState();
        ret=resp.substr(0,resp.length()-1)+",\"gamestate\":"+gamestate+"}";
        return ret;
    }
};

class MatchV1 : public MatchBase
{
public:
	//match object needs to have the abilities mapping, the map object can have additional abilities for custom maps though
	MatchV1(uint Version, string Format, string MapName) : MatchBase(Version, Format, MapName)
	{
		assert(version == 1);
	}
};
//match, matchsearch, userplayer, GamePage, GameHome, GameView, GameManageTeam, GameLeaderboard, CurrentMatches
typedef MatchV1 CurrentMatchVersion;

std::shared_ptr<MatchBase> CreateMatchObject(uint version, string format, string map_name DEBUGARGSDEF)
{
	if (version == 1) {
		return std::make_shared<MatchV1>(MatchV1(version, format, map_name));
	} else {
		version = CURRENT_MATCH_VERSION;
		return std::make_shared<CurrentMatchVersion>(CurrentMatchVersion(version, format, map_name));
	}
}

std::shared_ptr<MatchBase> CreateMatchObject(uint match_id, bool for_update DEBUGARGSDEF)
{
	vector<uint> user_ids;
	vector<uint> user_teams;
	vector<string> usernames;
	vector<int> users_money;
	vector<int> users_wonlost;
	string map_name;
	string format;
	uint version = 0;
	uint turn=0;
	uint turn_current_player=0;
	uint action_points=0;
	int active=0;

	string query = "select match_id,user_id,username,format,unix_timestamp(matches.created),map,turn,turn_current_player,team_id,version,action_points,money,matches.active,wonlost from matches join match_players using(match_id) join users using(user_id) where matches.match_id=" + ToString(match_id) + " order by team_id asc,user_id";
	if (for_update) query += " for update";
	auto * res = mysqlcon.select(query.c_str() DEBUGARGS);

	auto num_rows = mysql_num_rows(res);
	for (decltype(num_rows) r = 0; r < num_rows; r++) {
		auto row = mysqlcon.fetch_row(res);
		version = row[9];
		user_ids.push_back((uint)row[1]);
		usernames.push_back(row[2].ToString());
		user_teams.push_back(row[8]);
		format = row[3].cstr();
		map_name = row[5].cstr();
		turn = row[6];
		turn_current_player = row[7];
		action_points = row[10];
		users_money.push_back( (int)row[11]);
		active=row[12];
		users_wonlost.push_back( (int)row[13]);
	}

	auto match = CreateMatchObject(version, format, map_name DEBUGARGS);
	match->ReadInitQuery(match_id, active, user_ids, user_teams, usernames, users_money, users_wonlost, turn, turn_current_player, action_points, for_update DEBUGARGS);
	return match;
}

class MatchSearch
{
public:
	uint user_id, agesecs, mmr, mmrspread, created;
	string format;
	string screated;
	int made;

	MatchSearch()
	{
		made = 0;
	}
};

class MatchSearches : public Module
{
public:
	vector<MatchSearch> match_searches;
	bool for_update;
	
	MatchSearches() {
		for_update=false;
	}

	//start match - needs to make matches, match_players, and match_units
	//should I use my old stratacular model for modifying units?
	//on the destructor of the Match object? go through the units list and write a query to update all the modified ones
	size_t FindMatches(MatchSearch &m, uint find_players, vector<MatchSearch*> &tms)
	{
		tms.push_back(&m);
		for (auto &a : match_searches) {
			if (a.made) continue;
			if (a.format != m.format) continue;
			if (abs((int)m.mmr - (int)a.mmr) >= (int)m.mmrspread) continue;
			bool same = false;
			for (auto b : tms) {
				if (a.user_id == b->user_id) {
					same = true;
					break;
				}
			}
			if (same) continue;
			
			tms.push_back(&a);
			if (tms.size() == find_players) break;
		}
		if (tms.size() != find_players) return 0;
		/*for (auto p : tms) {
			p->made = 1;
			where_searches += "(user_id=" + ToString(p->user_id) + " AND format='" + mysqlcon.escape(p->format, 32) + "' AND created='" + mysqlcon.escape(p->screated, 64) + "') OR ";
		}*/
		return tms.size();
	}
	
	size_t MakeMatch(MatchSearch &m, uint find_players, vector<MatchSearch*> &tms, string &delete_searches, string &insert_match_players, string &insert_match_units)
	{
		if (tms.size() == find_players) {
			for (auto p : tms) {
				p->made = 1;
				delete_searches += "(user_id=" + ToString(p->user_id) + " AND format='" + mysqlcon.escape(p->format, 32) + "' AND created='" + mysqlcon.escape(p->screated, 64) + "') OR ";
			}
			
			//delete match_searches from DB in bulk, inserts for matches, match_players, match_units, and match_messages?
			//actually I should build the queries in megabulk, so it's not queries per match
			//the match insert needs to be done individually in order to get the match_id, lame
			string map_name = "Plains of War";
			uint match_id = (uint)mysqlcon.insert(("insert into matches set created=now(),active=1,version="+ToString(CURRENT_MATCH_VERSION)+",map='"+mysqlcon.escape(map_name, 64)+"',format='" + mysqlcon.escape(m.format, 32) + "',turn=1,action_points=0,turn_current_player=" + ToString(m.user_id)).c_str() DEBUGARGS);
			vector<UserAndTeam> match_users;
			vector<uint> user_teams;
			//for(auto p : tms) {
			for (uint i = 0; i<tms.size(); i++) {
				auto p = tms[i];
				uint team = i + 1;
				if (m.format == "2v2") team = (i / 2) + 1;
				user_teams.push_back(team);

				insert_match_players += "(" + ToString(match_id) + "," + ToString(p->user_id) + ",1,0,0," + ToString(team) + "),";
				if (p->user_id == current_user.user_id) {
					match_users.push_back(UserAndTeam(current_user));
				} else {
					User tuser;
					tuser.user_id = p->user_id;
					match_users.push_back(UserAndTeam(tuser));
				}
			}
			//auto units = Map().InitMatchUnits(match_users, user_teams);
			auto mo = CreateMatchObject(CURRENT_MATCH_VERSION, m.format, map_name DEBUGARGS);
			auto units = mo->map->InitMatchUnits(match_users, user_teams);
			for (auto u : units) {
				insert_match_units += u.MysqlInsertValues(match_id) + ",";
			}
		} else return 0;
		
		return tms.size();
	}

	virtual void CheckMakeMatches()
	{
		for_update=false;
		GetData(DEBUGARG);
		
		size_t total_found=0;
		string where_searches;
		for (auto &m : match_searches) {
			if(m.made) continue;
			uint find_players = 2;
			if (m.format == "2v2" || m.format == "ffa") find_players = 4;
			vector<MatchSearch*> tms;
			total_found+=FindMatches(m, find_players, tms);
		}
		
		if(total_found) for_update=true;
		else return;
		
		gamesimheader.GetData(DEBUGARG);
		if(simheader.samples<HISTORY_MIN_SIZE) {
			simheader.isloaded=false;//these only get loaded if it finds a match anyways, not a terrible loss
			gamesimheader.isloaded=false;
			for_update=false;
			return;
		}
		
		isloaded=false;
		match_searches.clear();
		GetData(DEBUGARG);
		
		total_found=0;
		string delete_searches;//="delete from match_searches where ";//(sdf=fds and sdgfds=1) OR
		string insert_match_players;//="insert into match_players(match_id,user_id,active,last_mid_read,last_alerted_turn) values";
		string insert_match_units;//="insert into match_units(match_id,user_id,unit_type,hp,mana,damage,xpos,ypos,data1,data2) values";
		for (auto &m : match_searches) {
			if (m.made) continue;
			uint find_players = 2;
			if (m.format == "2v2" || m.format == "ffa") find_players = 4;
			vector<MatchSearch*> tms;
			total_found+=FindMatches(m, find_players, tms);
			MakeMatch(m, find_players, tms, delete_searches, insert_match_players, insert_match_units);
		}
		if (delete_searches.length()) {
			delete_searches = "delete from match_searches where " + delete_searches.substr(0, delete_searches.length() - 4);
			mysqlcon.delete_query(delete_searches.c_str() DEBUGARGS);
		}
		if (insert_match_players.length()) {
			insert_match_players = "insert into match_players(match_id,user_id,active,last_mid_read,last_alerted_turn,team_id) values" + insert_match_players.substr(0, insert_match_players.length() - 1);
			mysqlcon.insert(insert_match_players.c_str() DEBUGARGS);
		}
		if (insert_match_units.length()) {
			insert_match_units = "insert into match_units(match_id,user_id,unit_type,maxhp,hp,maxmana,mana,damage,xpos,ypos,data1,data2,cooldown,speed,attack_range,armor,isdef) values" + insert_match_units.substr(0, insert_match_units.length() - 1);
			mysqlcon.insert(insert_match_units.c_str() DEBUGARGS);
		}
		
		simheader.isloaded=false;//these only get loaded if it finds a match anyways, not a terrible loss, also running at the end means these shouldn't be reloaded anyways
		gamesimheader.isloaded=false;
	}
	
	virtual void RunQuery()
	{
		string query="select user_id, timestampdiff(microsecond,created,now(6))/1000000 as agesecs, format, mmr, mmrspread, created, unix_timestamp(created) from match_searches order by user_id=" + ToString(current_user.user_id) + " desc,created asc limit 100";
		if(for_update) query += " for update";
		auto * res = mysqlcon.select(query.c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			MatchSearch ms;
			ms.user_id = row[0];
			ms.agesecs = row[1];
			ms.format = ToLower(row[2].cstr());
			ms.mmr = row[3];
			ms.mmrspread = row[4];
			ms.mmrspread = ms.agesecs + 600;
			ms.screated = row[5].cstr();
			ms.created = row[6];
			match_searches.push_back(ms);
		}
	}

	virtual string InnerHtml()
	{
		string ret = "<div class='match-searches'>";
		current_user.GetData(DEBUGARG);
		for (auto &m : match_searches) {
			if (m.made == 0 && m.user_id == current_user.user_id) {
				string f = m.format;
				if (f == "ffa") f = "FFA";
				ret += "<div class='match-search'><i class='fa fa-times cancel-match-search' format='"+mysqlcon.escape(f,32)+"' created='"+ToString(m.created)+"'></i> Looking for a " + f + " match since " + ToString(m.agesecs/60) + " minutes ago.</div>";
			}
		}
		ret += "</div>";
		return ret;
	}
};
MatchSearches matchsearches;


class MatchInfo
{
public:
	uint match_id;
	vector<uint> user_ids;
	vector<string> usernames;
	string format;
	uint created;
	string map;
	uint turn;
	uint turn_current_player;
};

class CurrentMatches : public Module
{
public:
	vector<MatchInfo> matches;

	virtual void RunQuery()
	{
		auto * res = mysqlcon.select(("select match_id,user_id,username,format,unix_timestamp(matches.created),map,turn,turn_current_player from matches join match_players using(match_id) join users using(user_id) where matches.active=1 and match_id in(select match_id from match_players where match_players.active=1 and user_id=" + ToString(current_user.user_id) + ")").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint match_id = row[0];
			if (matches.size() == 0 || matches.back().match_id != match_id) {
				matches.push_back(MatchInfo());
			}
			MatchInfo &m = matches.back();
			m.match_id = match_id;
			m.user_ids.push_back((uint)row[1]);
			m.usernames.push_back(row[2].ToString());
			m.format = row[3].cstr();
			m.created = row[4];
			m.map = row[5].cstr();
			m.turn = row[6];
			m.turn_current_player = row[7];
		}
	}

	virtual string InnerHtml()
	{
		string ret = "<div class='match-searches'>";
		current_user.GetData(DEBUGARG);
		for (auto &m : matches) {
			string usernames;
			string turn_username;
			for (uint i = 0; i<m.usernames.size(); i++) {
				User tu;
				tu.username = m.usernames[i];
				tu.user_id = m.user_ids[i];
				string u = tu.namehtml();
				if (m.user_ids[i] == m.turn_current_player) turn_username = u = "<span class='username username-turn'>" + u + "</span>";
				else u = "<span class='username'>" + u + "</span>";
				if (i + 1 == m.usernames.size()) usernames += "and " + u;
				else if (m.usernames.size() == 2) usernames += u + " ";
				else usernames += u + ", ";
			}
			string classes = "current-match";
			string f = m.format;
			if (f == "ffa") f = "FFA";
			if (m.turn_current_player == current_user.user_id) classes += " my-turn";
			ret += "<div class='" + classes + "'><a href='" + MakeURL("page=gameview&matchid=" + ToString(m.match_id)) + "'>" + f + " Match ID " + ToString(m.match_id) + "</a> with " + usernames + " on " + m.map + ". It is " + turn_username + "'s turn, on turn #"+ToString(m.turn)+".</div>";
		}
		ret += "</div>";
		return ret;
	}
};
CurrentMatches currentmatches;

class Leaderboard : public Module
{
public:
	vector<UserAndTeam> users;
	virtual void RunQuery()
	{
		gamesimheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
		//users.push_back(User());
		uint last_user_id=0;
		auto * res = mysqlcon.select(("select user_id,player_id,bought_for,username,wcs_points from users_players join users using(user_id) where active=1 order by user_id,player_id") DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			UserPlayer up;
			up.active = 1;

			auto row = mysqlcon.fetch_row(res);
			uint user_id = row[0];
			up.player_id = row[1];
			up.bought_for = row[2];
			up.headband = players.players[up.player_id].headband;
			if (user_id != last_user_id) {
				users.push_back(User());
				users.back().u.user_id = user_id;
				users.back().u.username = row[3].cstr();
				users.back().u.wcspoints = row[4];
			}
			last_user_id=user_id;
			UserAndTeam &u = users.back();
			u.u.isloaded = true;
			u.t.isloaded = true;
			u.t.team.push_back(up);
		}
		for (auto &u : users) u.t.CalcTeamValue();
		//users[0] = users.back();
		//users.pop_back();
		sort(users.begin(), users.end(), [](const UserAndTeam &a, const UserAndTeam &b) {
			if(a.t.teamvalue+a.u.wcspoints == b.t.teamvalue+b.u.wcspoints) {
				return a.t.teamvalue>b.t.teamvalue;
			}
			return a.t.teamvalue+a.u.wcspoints>b.t.teamvalue+b.u.wcspoints;
		});
	}

	string InnerHtml()
	{
		string me;
		string ret;
		ShowMore showmore;
		for (uint i = 0; i<users.size(); i++) {
			auto &u = users[i].u;
			auto &t = users[i].t;
			string splayers;
			for (uint a = 0; a<t.team.size(); a++) {
				auto &p = players.players[t.team[a].player_id];
				string c = "perc style='color:black;' title='Bought for " + ToString(t.team[a].bought_for) + "'";
				if (t.team[a].bought_for-25 > PlayerValue(p.player_id)) c = "bperc title='Bought for "+ToString(t.team[a].bought_for)+"'";
				else if (t.team[a].bought_for+25 < PlayerValue(p.player_id)) c = "gperc title='Bought for " + ToString(t.team[a].bought_for) + "'";
				splayers += players.pshorthtml(p.player_id) + " (<"+c+">" + ToString(PlayerValue(p.player_id)) + "</"+c+">), ";
			}
			if (splayers.length()) splayers += "and ";
			splayers += ToString(u.wcspoints)+" unspent points.";
			//if (splayers.length()) splayers = splayers.substr(0, splayers.length() - 2);
			//if(i==3) ret+="<hr></hr>";
			if (i == 20) ret += showmore.Start("Users");
			string line = "<mdiv>";
			//if (u.user_id == current_user.user_id) line = "<mdiv style=':#88f 1px solid;'>";
			//else line = "<mdiv>";
			line += "#" + ToString(i + 1) + " " + u.namehtml() + "'s team is worth " + ToStringCommas(t.teamvalue + u.wcspoints) + " WCS Points with " + splayers + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>\n";
			ret += line;
			if (u.user_id == current_user.user_id) me = line+"<rhr></rhr>";
		}
		ret += showmore.End();
		return me+ret;
	}
};
Leaderboard leaderboard;

struct TeamHistoryEvent
{
	uint uid, bought_sold, timestamp, amount, new_wcs_points, player_id, new_team_value;
	TeamHistoryEvent(uint UID, uint BoughtOrSold, uint Timestamp, uint Amount, uint PlayerID)
	{
		uid = UID;
		bought_sold = BoughtOrSold;
		timestamp = Timestamp;
		amount = Amount;
		new_wcs_points = 0;
		new_team_value = 0;
		player_id = PlayerID;
	}
};

class TeamValueHistory : public Module
{
public:
	uint user_id;
	uint curr_wcs_points;
	uint starting_wcs_points;
	string username;

	vector<TeamHistoryEvent> events;
	string histories;
	
	TeamValueHistory(uint USER_ID)
	{
		user_id=USER_ID;
	}
	
	virtual void RunQuery()
	{
		gamesimheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
		starting_wcs_points = wwglobals.starting_wcs_points;

		uint twcs_points = starting_wcs_points;

		auto * res = mysqlcon.select(("select player_id,bought_for,sold_for,wcs_points,active,unix_timestamp(bought_on),unix_timestamp(sold_on),username from users_players join users using(user_id) where user_id=" + ToString(user_id) + " and bought_on between from_unixtime(" + ToString(wwglobals.season_start) + ") and from_unixtime(" + ToString(wwglobals.season_end) + ") order by bought_on").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			UserPlayer up;
			auto row = mysqlcon.fetch_row(res);
			//uint user_id = user_id;
			up.player_id = row[0];
			up.bought_for = row[1];
			up.sold_for = row[2];
			curr_wcs_points = row[3];
			up.active = row[4];
			up.bought_on = row[5];
			up.sold_on = row[6];
			username = row[7].cstr();
			//up.headband = players.players[up.player_id].headband;

			if (up.sold_on>0 && up.sold_for == 0) {
				events.clear();
				continue;
			}

			uint bought_sold = 0;
			TeamHistoryEvent ev(user_id, bought_sold, up.bought_on, up.bought_for, up.player_id);
			events.push_back(ev);
			if (up.sold_for > 0) {
				bought_sold = 1;
				TeamHistoryEvent evs(user_id, bought_sold, up.sold_on, up.sold_for, up.player_id);
				events.push_back(evs);
			}
		}
		sort(events.begin(), events.end(), [](const TeamHistoryEvent &a, const TeamHistoryEvent &b) {
			return a.timestamp < b.timestamp;
		});
		uint first_timestamp = (uint)time(0);
		uint end_timestamp = wwglobals.season_end;// (uint)time(0) + 86400;
		string spids = "";

		for (auto &e : events) {
			if (e.bought_sold == 0) twcs_points -= e.amount;
			else if (e.bought_sold == 1) twcs_points += e.amount;
			e.new_wcs_points = twcs_points;
			first_timestamp = min(first_timestamp, e.timestamp);
			if (e.bought_sold == 0) spids += ToString(e.player_id) + ",";
		}
		if (spids.length() > 0) spids = spids.substr(0, spids.length() - 1);
		else return;
		first_timestamp -= 86400 * 5;
		first_timestamp = max(first_timestamp, wwglobals.season_start);
		res = mysqlcon.select(("select players_wcs_points.p_id, sims.sim_id, sims.size, unix_timestamp(sims.created), min, median from players_wcs_points join sims using(sim_id) where players_wcs_points.p_id in(" + spids + ") and sims.created>'2014-11-01' and sims.created between from_unixtime(" + ToString(first_timestamp) + ")-interval 1 second and from_unixtime(" + ToString(end_timestamp) + ")+interval 1 second and (sims.size>=" + ToString(HISTORY_MIN_SIZE) + " or players_wcs_points.sim_id=" + ToString(simheader.sim_id) + " or players_wcs_points.sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac=" + ToString(USE_ALIGULAC) + " and (sims.type=" + ToString(SIM_TYPE) + " or sims.sim_id=" + ToString(simheader.sim_id) + ") order by created").c_str() DEBUGARGS);

		string history = "['" + username + "',";
		//uint last_pid = 0;
		uint last_wcs_points = starting_wcs_points;
		int tvalue = (int)starting_wcs_points;
		int last_value = tvalue;
		uint last_sim_id = 0;
		uint last_created = first_timestamp - 86400;
		array<bool, MAX_PLAYER_ID> actives = { { 0 } };
		array<uint, MAX_PLAYER_ID> last_mins = { { 0 } };
		array<uint, MAX_PLAYER_ID> last_medians = { { 0 } };
		uint event_slot = 0;
		int gift = 0;
		
		num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint sim_id=row[1];
			//uint sim_size=row[2];
			uint created=row[3];
			uint min_points=row[4];
			uint median_points=row[5];
			if(sim_id!=last_sim_id) {
				if (last_sim_id > 0) {
					//if (last_sim_id == 2717) tvalue = last_value;
					if (last_sim_id == 2717 || last_sim_id == 3219 || last_sim_id == 3228 || last_sim_id == 4117) gift += (int)last_value - (int)tvalue;
					history += ToString(tvalue+gift) + "," + ToString(last_created) + ",";
				}
				while (event_slot < events.size() && created >= events[event_slot].timestamp) {
					auto &e = events[event_slot];
					last_wcs_points = e.new_wcs_points;
					if (e.bought_sold == 0) actives[e.player_id] = true;
					else if (e.bought_sold == 1) actives[e.player_id] = false;
					event_slot++;
					tvalue = last_wcs_points;
					for (uint pid = 1; pid < actives.size(); pid++) {
						if (actives[pid]) tvalue += PointsValue(last_mins[pid], last_medians[pid]);
					}
					e.new_team_value = tvalue;
					last_created = e.timestamp;
					history += ToString(tvalue+gift) + "," + ToString(last_created) + ",";
				}
				last_value = tvalue;
				tvalue = last_wcs_points;
				last_sim_id = sim_id;
				last_created = created;
			}
			last_mins[pid] = min_points;
			last_medians[pid] = median_points;
			if(actives[pid]) tvalue+=PointsValue(min_points, median_points);
		}
		if (last_sim_id > 0) {
			if (last_sim_id == 2717 || last_sim_id == 3219 || last_sim_id == 3228 || last_sim_id == 4117) gift += (int)last_value - (int)tvalue;
			history += ToString(tvalue+gift) + "," + ToString(last_created) + ",";
		}
		while (event_slot < events.size()) {
			auto &e = events[event_slot];
			last_wcs_points = e.new_wcs_points;
			if (e.bought_sold == 0) actives[e.player_id] = true;
			else if (e.bought_sold == 1) actives[e.player_id] = false;
			event_slot++;
			tvalue = last_wcs_points;
			for (uint pid = 1; pid < actives.size(); pid++) {
				if (actives[pid]) tvalue += PointsValue(last_mins[pid], last_medians[pid]);
			}
			e.new_team_value = tvalue;
			last_created = e.timestamp;
			history += ToString(tvalue+gift) + "," + ToString(last_created) + ",";
		}
		if(history.length()<=1) return;
		history+="]";
		histories=history;
	}
	
	string InnerHtml()
	{
		string ret;
		string team_events;
		for(auto &e : events) {
			auto &p = players.players[e.player_id];
			//ret += players.pshorthtml(p.player_id) + " ("+ToString(e.new_wcs_points)+"), ";
			team_events += "{c:"+ToString(e.timestamp)+",bought_sold:"+ToString(e.bought_sold)+", amount:"+ToString(e.amount)+", pid:"+ToString(e.player_id)+", player:'"+jsonescape(p.name.ToString(),256)+"'},";
		}
		if (team_events.length()) {
			team_events = team_events.substr(0, team_events.length() - 1);
			ret += "\n<script>window.wcs.team_events.push(" + team_events + ");</script>\n";
		}
		ret += "<div class='graph team-history-graph' data-uids='"+ToString(user_id)+"'></div>";
		ret += "\n<script>window.wcs.team_historiesa["+ToString(user_id)+"]=("+histories+");</script>\n";
		return ret;
	}

	string TradeHistoryHtml(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		array<int, MAX_PLAYER_ID> bought_fors = { { 0 } };

		for (uint i = 0; i < events.size();i++) {
			if (i == 10) ret += showmore.Start("Trade History");
			auto &e = events[i/*events.size()-i-1*/];
			auto &p = players.players[e.player_id];
			int profit = 0;
			if (e.bought_sold == 1) {
				profit = (int)e.amount - bought_fors[p.player_id];
			}
			ret += "<mdiv>" + players.phtml(p.player_id) + (e.bought_sold == 0 ? " bought" : " sold") + " for " + ToString(e.amount) + " points" + (e.bought_sold == 1 ? "(" + ToString(profit) + ") " : " ") + "on " + TimestampHtml(e.timestamp) + ".</mdiv>";
			if (e.bought_sold == 0) bought_fors[p.player_id] = (int)e.amount;
		}
		ret += showmore.End();
		return ret;
	}
};

class AllTrades : public Module
{
public:
	vector<vector<TeamHistoryEvent>> users_events;
	vector<uint> last_resets;
	vector<string> usernames;

	virtual void RunQuery()
	{
		gamesimheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
		uint twcs_points = wwglobals.starting_wcs_points;

		auto * res = mysqlcon.select(("select player_id,bought_for,sold_for,wcs_points,active,unix_timestamp(bought_on),unix_timestamp(sold_on),username,user_id from users_players join users using(user_id) where bought_on between from_unixtime("+ToString(wwglobals.season_start)+") and from_unixtime("+ToString(wwglobals.season_end)+") order by bought_on").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			UserPlayer up;
			auto row = mysqlcon.fetch_row(res);
			//uint user_id = user_id;
			up.player_id = row[0];
			up.bought_for = row[1];
			up.sold_for = row[2];
			//uint curr_wcs_points = row[3];
			up.active = row[4];
			up.bought_on = row[5];
			up.sold_on = row[6];
			string username = row[7].cstr();
			uint user_id = row[8];
			//up.headband = players.players[up.player_id].headband;

			/*if (up.sold_on>0 && up.sold_for == 0) {
				events.clear();
				continue;
				}*/

			if (user_id >= users_events.size()) {
				users_events.resize(user_id + 16);
				usernames.resize(user_id + 16);
				last_resets.resize(user_id + 16);
			}
			usernames[user_id] = username;
			auto &events = users_events[user_id];

			uint bought_sold = 0;
			TeamHistoryEvent ev(user_id, bought_sold, up.bought_on, up.bought_for, up.player_id);
			events.push_back(ev);
			if (up.sold_on > 0) {
				bought_sold = 1;
				TeamHistoryEvent evs(user_id, bought_sold, up.sold_on, up.sold_for, up.player_id);
				events.push_back(evs);
			}
		}
		for (auto &events : users_events) {
			sort(events.begin(), events.end(), [](const TeamHistoryEvent &a, const TeamHistoryEvent &b) {
				return a.timestamp < b.timestamp;
			});
		}
		for (auto &events : users_events) {
			for (uint i = 0; i < events.size();i++) {
				auto &e = events[i];
				if (e.bought_sold == 1 && e.amount == 0) last_resets[e.uid] = i+1;
			}
		}
		uint first_timestamp = (uint)time(0);
		//uint end_timestamp = wwglobals.season_end;// (uint)time(0) + 86400;
		string spids = "";

		for (auto &events : users_events) {
			for (auto &e : events) {
				if (e.bought_sold == 0) twcs_points -= e.amount;
				else if (e.bought_sold == 1) twcs_points += e.amount;
				e.new_wcs_points = twcs_points;
				first_timestamp = min(first_timestamp, e.timestamp);
				if (e.bought_sold == 0) spids += ToString(e.player_id) + ",";
			}
		}
		if (spids.length()>0) spids = spids.substr(0, spids.length() - 1);
		else return;
		first_timestamp -= 86400 * 5;
	}

	string InnerHtml()
	{
		string ret;
		return ret;
	}
};
AllTrades all_trades;

class BestWorstTrades : public Module
{
public:
	vector<std::pair<TeamHistoryEvent, TeamHistoryEvent> > events;

	virtual void RunQuery()
	{
		all_trades.GetData(DEBUGARG);
		for (auto &evs : all_trades.users_events) {
			uint start = 0;
			if(evs.size()) start = all_trades.last_resets[evs[0].uid];
			for (uint i = start; i < evs.size(); i++) {
				auto &e = evs[i];
				if (e.bought_sold != 0) continue;
				for (uint a = i + 1; a < evs.size(); a++) {
					auto &e2 = evs[a];
					if (e2.bought_sold != 1) continue;
					if (e2.uid != e.uid) continue;
					if (e2.player_id != e.player_id) continue;

					events.push_back(std::make_pair(e, e2));
					break;
				}
			}
		}
		sort(events.begin(), events.end(), [](const std::pair<TeamHistoryEvent, TeamHistoryEvent> &a, const std::pair<TeamHistoryEvent, TeamHistoryEvent> &b) {
			int profit_a = (int)a.second.amount - (int)a.first.amount;
			int profit_b = (int)b.second.amount - (int)b.first.amount;
			return profit_a > profit_b;
		});
	}

	string InnerHtml()
	{
		return string("Don't use this");
	}

	string TradeLine(uint rank, std::pair<TeamHistoryEvent, TeamHistoryEvent> &ev)
	{
		string ret;
		User u;
		u.user_id = ev.first.uid;
		u.username = all_trades.usernames[u.user_id];
		u.isloaded = true;
		int profit = (int)ev.second.amount - (int)ev.first.amount;
		string sprofit = "gained " + ToString(profit);
		if (profit < 0) sprofit = "lost " + ToString(profit*-1);
		ret = "<mdiv>#" + ToString(rank) + " " + u.namehtml() + " " + sprofit + " points from "+players.phtml(ev.first.player_id)+", bought on "+TimestampHtml(ev.first.timestamp)+" for "+ToString(ev.first.amount)+" points, sold on "+TimestampHtml(ev.second.timestamp)+" for "+ToString(ev.second.amount)+" points."+(globalcontext.r==Renderer::bbcode?"<br/><br/>":"")+"</mdiv>";
		return ret;
	}

	string BestTrades(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < events.size() && i<50; i++) {
			if (i == 5) ret += showmore.Start("Best Trades");
			auto &ev = events[i];
			int profit = (int)ev.second.amount - (int)ev.first.amount;
			if (profit < 0) break;
			ret += TradeLine(i + 1, ev);
		}
		ret += showmore.End();
		return ret;
	}

	string WorstTrades(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < events.size() && i<50; i++) {
			if (i == 5) ret += showmore.Start("Worst Trades");
			auto &ev = events[events.size() - i - 1];
			int profit = (int)ev.second.amount - (int)ev.first.amount;
			if (profit > 0) break;
			ret += TradeLine(i + 1, ev);
		}
		ret += showmore.End();
		return ret;
	}
};
BestWorstTrades best_worst_trades;

struct ChangeSpan
{
	uint pid;
	uint start_slot, end_slot;
	int start_val, end_val;
};
class PlayersValueHistory : public Module
{
public:
	vector<std::pair<uint, vector<uint> > > players_values;
	vector<uint> timestamps;
	vector<ChangeSpan> change_spans;
	vector<ChangeSpan> change_spans_full;

	virtual void RunQuery()
	{
		gamesimheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);

		array<vector<uint>, MAX_PLAYER_ID> tplayers_values;
		uint first_timestamp = wwglobals.season_start;
		uint end_timestamp = wwglobals.season_end;
		uint last_sim_id = 0;
		auto res = mysqlcon.select(("select players_wcs_points.p_id, sims.sim_id, sims.size, unix_timestamp(sims.created), min, median from players_wcs_points join sims using(sim_id) where sims.created>'2014-11-01' and sims.created between from_unixtime(" + ToString(first_timestamp) + ")-interval 1 second and from_unixtime(" + ToString(end_timestamp) + ")+interval 1 second and (sims.size>=" + ToString(HISTORY_MIN_SIZE) + " or players_wcs_points.sim_id=" + ToString(simheader.sim_id) + " or players_wcs_points.sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac=" + ToString(USE_ALIGULAC) + " and (sims.type=" + ToString(SIM_TYPE) + " or sims.sim_id=" + ToString(simheader.sim_id) + ") order by created asc").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint sim_id = row[1];
			//uint sim_size = row[2];
			uint created = row[3];
			uint min_points = row[4];
			uint median_points = row[5];
			uint value = PointsValue(min_points, median_points);
			if (sim_id != last_sim_id) {
				timestamps.push_back(created);
				last_sim_id = sim_id;
			}
			tplayers_values[pid].resize(timestamps.size());
			tplayers_values[pid].back() = value;
		}
		for (uint i = 0; i < tplayers_values.size(); i++) {
			auto &p = tplayers_values[i];
			uint min_val = 999999;
			uint max_val = 0;
			for (auto v : p) {
				//v = max(v, wwglobals.base_cost);//should I write this back? I think it's important to distinguish when a player didn't "exist"
				if (v >= (uint)wwglobals.base_cost) {
					min_val = min(min_val, v);
					max_val = max(max_val, v);
				}
			}
			if (min_val == 999999 && max_val == 0) continue;
			if (min_val + 25 >= max_val) continue;
			p.resize(timestamps.size());
			players_values.push_back(std::make_pair(i, p));
		}
		for (auto &p : tplayers_values) p.clear();

		uint span_width = 180*86400;//days
		const int c_min_change = 250;//or should this be by percent? a player going from 200 points to 400 points doesn't sound like a great buy to me, a player going from 2000 points to 4000 points sounds amazing though
		for (auto &p : players_values) {
			//uint last_end = 0;
			//int last_change = 0;
			uint start_slot = (uint)change_spans.size();

			for (uint s = 0; s < timestamps.size(); s++) {
				uint start = timestamps[s];
				int sval = (int)p.second[s];
				if (sval == 0) continue;
				int max_change = (c_min_change-1) * 1;
				int min_change = (c_min_change - 1) * -1;
				ChangeSpan span;

				for (uint e = s + 1; e < timestamps.size(); e++) {
					uint end = timestamps[e];
					if (end - start > span_width) break;
					int eval = (int)p.second[e];
					//if (end < wwglobals.season_end) end = end*wwglobals.sell_percent / 100;
					int age_penalty = ((int)(end - start)) * 1 / 86400;
					//int change = std::abs(eval - sval - age_penalty);
					int change = eval - sval - age_penalty;
					//change -= ((int)(end - start)) * 2 / 86400;//each day costs 2 points? a month costs 60 points?

					//if (change > max_change) {
					//if (change >= min_change)
					if (change<=c_min_change*-1 || change >= c_min_change) {
						ChangeSpan tspan;
						tspan.start_slot = s;
						tspan.end_slot = e;
						tspan.pid = p.first;
						tspan.start_val = sval;
						tspan.end_val = eval;
						change_spans_full.push_back(tspan);
					}
					if (change<min_change*9/10 || change>max_change*9/10) {
						span.start_slot = s;
						span.end_slot = e;
						span.pid = p.first;
						span.start_val = sval;
						span.end_val = eval;
						max_change = max(max_change,change);
						min_change = min(min_change, change);
						change_spans.push_back(span);
					}
				}
				/*if (max_change < min_change) continue;
				if (last_end == span.end_slot) {
					if (last_change > max_change) continue;
					//change_spans.pop_back();
				}
				//change_spans.push_back(span);
				last_end = span.end_slot;
				last_change = max_change;*/
			}
			sort(change_spans.begin() + start_slot, change_spans.end(), [this](const ChangeSpan &a, const ChangeSpan &b) {
				uint start_a = timestamps[a.start_slot];
				uint end_a = timestamps[a.end_slot];
				int age_penalty_a = ((int)(end_a - start_a)) * 1 / 86400;
				int change_a = std::abs(a.end_val - a.start_val - age_penalty_a);

				uint start_b = timestamps[b.start_slot];
				uint end_b = timestamps[b.end_slot];
				int age_penalty_b = ((int)(end_b - start_b)) * 1 / 86400;
				int change_b = std::abs(b.end_val - b.start_val - age_penalty_b);

				return change_a > change_b;
			});
			for (uint i = start_slot; i < change_spans.size(); i++) {
				auto &a = change_spans[i];
				for (uint j = i+1; j < change_spans.size(); j++) {
					auto &b = change_spans[j];
					if (a.end_val>a.start_val && b.end_val < b.start_val) continue;//different direction!
					if (a.end_val<a.start_val && b.end_val > b.start_val) continue;//different direction!
					if (a.end_slot>b.start_slot && a.start_slot<b.end_slot) {
						change_spans.erase(change_spans.begin()+j);
						j--;
					}
				}
			}
		}
		stable_sort(change_spans.begin(), change_spans.end(), [](const ChangeSpan &a, const ChangeSpan &b) {
			return a.end_val - a.start_val > b.end_val - b.start_val;
		});
		stable_sort(change_spans_full.begin(), change_spans_full.end(), [](const ChangeSpan &a, const ChangeSpan &b) {
			return a.end_val - a.start_val > b.end_val - b.start_val;
		});
	}

	string Line(uint rank, ChangeSpan span)
	{
		string ret;
		uint start_time = timestamps[span.start_slot];
		uint end_time = timestamps[span.end_slot];
		uint ispan = end_time - start_time;
		string sspan = ToString(ispan / 86400) + " days";
		if (ispan < 86400 * 3) sspan = ToString(ispan / 3600) + " hours";
		ret = "<mdiv>#" + ToString(rank) + " " + players.phtml(span.pid) + " gained " + ToString(span.end_val - span.start_val) + " points over " + sspan;
		ret += ", going from " + ToString(span.start_val) + " points on " + TimestampHtml(start_time) + ", to " + ToString(span.end_val) + " points on " + TimestampHtml(end_time) + "." + (globalcontext.r==Renderer::bbcode?"<br/><br/>":"")+"</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ret = "<mdiv>change_spans.size() == " + ToString(change_spans.size())+", change_spans_full.size() == "+ToString(change_spans_full.size())+"</mdiv>";
		uint rank = 1;
		ShowMore showmore;
		for (auto &c : change_spans) {
			if (rank > 25) break;
			//if (c.pid != 5) continue;
			//if (timestamps[c.end_slot] < wwglobals.season_end-86400-4000) continue;
			if (rank == 6) ret += showmore.Start("Best Possible Trades");
			//ret += "<mdiv>"+players.phtml(c.pid) + " changed by " + ToString(c.end_val - c.start_val) + " over a span of " + ToString((timestamps[c.end_slot] - timestamps[c.start_slot]) / 86400) + " days</mdiv>";
			ret += Line(rank, c);
			rank++;
		}
		ret += showmore.End();
		return ret;
	}
};
PlayersValueHistory players_value_history;

class QuickestProfitableTrades : public Module
{
public:
	vector<std::pair<TeamHistoryEvent, TeamHistoryEvent> > events;

	virtual void RunQuery()
	{
		best_worst_trades.GetData(DEBUGARG);
		//events = best_worst_trades.events;
		for (auto &e : best_worst_trades.events) {
			if (e.second.amount > e.first.amount) events.push_back(e);
		}
		sort(events.begin(), events.end(), [](const std::pair<TeamHistoryEvent, TeamHistoryEvent> &a, const std::pair<TeamHistoryEvent, TeamHistoryEvent> &b) {
			int span_a = (int)a.second.timestamp - (int)a.first.timestamp;
			int span_b = (int)b.second.timestamp - (int)b.first.timestamp;
			return span_a < span_b;
		});
	}

	string TradeLine(uint rank, std::pair<TeamHistoryEvent, TeamHistoryEvent> &ev)
	{
		string ret;
		User u;
		u.user_id = ev.first.uid;
		u.username = all_trades.usernames[u.user_id];
		u.isloaded = true;
		int profit = (int)ev.second.amount - (int)ev.first.amount;
		string sprofit = "gained " + ToString(profit);
		if (profit < 0) sprofit = "lost " + ToString(profit*-1);
		int ispan = (int)ev.second.timestamp - (int)ev.first.timestamp;
		string span=ToString(ispan/3600)+" hours";
		if (ispan > 86400 * 2) span = ToString(ispan / 86400) + " days";
		ret = "<mdiv>#" + ToString(rank) + " " + u.namehtml() + " " + sprofit + " points from " + players.phtml(ev.first.player_id) + " over a span of " + span + ", bought on " + TimestampHtml(ev.first.timestamp) + " for " + ToString(ev.first.amount) + " points, sold on " + TimestampHtml(ev.second.timestamp) + " for " + ToString(ev.second.amount) + " points." + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < events.size() && i<100; i++) {
			if (i == 10) ret += showmore.Start("Quickest Profits");
			auto &ev = events[i];
			ret += TradeLine(i + 1, ev);
		}
		ret += showmore.End();
		return ret;
	}
};

class GiversTakers : public Module
{
public:
	struct PlayerGiving {
		uint pid;
		int points;
		uint trades;
	};
	vector<PlayerGiving> players_givings;

	virtual void RunQuery()
	{
		best_worst_trades.GetData(DEBUGARG);
		array<int, MAX_PLAYER_ID> pplayers = { { 0 } };
		array<int, MAX_PLAYER_ID> players_trades = { { 0 } };
		for (auto &e : best_worst_trades.events) {
			int profit = (int)e.second.amount - (int)e.first.amount;
			pplayers[e.first.player_id] += profit;
			players_trades[e.first.player_id]++;
		}
		for (uint i = 0; i < pplayers.size(); i++) {
			PlayerGiving g;
			g.pid = i;
			g.points = pplayers[i];
			g.trades = players_trades[i];
			if (pplayers[i] != 0) players_givings.push_back(g);
		}
		std::sort(players_givings.begin(), players_givings.end(), [](const PlayerGiving &a, const PlayerGiving &b) {
			return a.points > b.points;
		});
	}

	string InnerHtml()
	{
		return string("Don't use this");
	}

	string Line(uint rank, PlayerGiving g)
	{
		string ret;
		ret += "<mdiv>#" + ToString(rank) + " " + players.phtml(g.pid) + (g.points >= 0 ? " gave " : " took ") + ToString(std::abs(g.points)) + " points with " + ToString(g.trades) + (g.trades != 1 ? " trades" : " trade") + ", for an average of " + ToString(g.points / (int)g.trades) + " points per trade." + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string Givers(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < players_givings.size() && i<25; i++) {
			if (i == 5) ret += showmore.Start("Givers");
			auto &g = players_givings[i];
			if (g.points < 0) break;
			ret += Line(i + 1, g);
		}
		ret += showmore.End();
		return ret;
	}

	string Takers(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < players_givings.size() && i<25; i++) {
			if (i == 5) ret += showmore.Start("Takers");
			auto &g = players_givings[players_givings.size() - i - 1];
			if (g.points > 0) break;
			ret += Line(i + 1, g);
		}
		ret += showmore.End();
		return ret;
	}
};

class MostSales : public Module
{
public:
	vector<std::pair<uint,uint> > users;

	virtual void RunQuery()
	{
		all_trades.GetData(DEBUGARG);
		best_worst_trades.GetData(DEBUGARG);
		vector<uint> tusers;
		for (auto &e : best_worst_trades.events) {
			if (e.first.uid >= tusers.size()) tusers.resize(e.first.uid + 16);
			tusers[e.first.uid]++;
		}
		for (uint i = 0; i < tusers.size(); i++) {
			if (tusers[i]>0) users.push_back(std::make_pair(i, tusers[i]));
		}
		sort(users.begin(), users.end(), [](const std::pair<uint,uint> &a, const std::pair<uint,uint> &b){
			return a.second > b.second;
		});
	}

	string Line(uint rank, uint uid, uint sales)
	{
		string ret;
		User u;
		u.user_id = uid;
		u.username = all_trades.usernames[uid];
		u.isloaded = true;
		ret = "<mdiv>#" + ToString(rank) + " " + u.namehtml() + " with " + ToString(sales) + (sales != 1 ? " sales " : " sale ") + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < users.size() && i<25; i++) {
			if (users[i].second <= 1) break;
			if (i == 5) ret += showmore.Start("Most Sales");
			ret += Line(i+1, users[i].first, users[i].second);
		}
		ret += showmore.End();
		return ret;
	}
};

class MostBought : public Module
{
public:
	vector<std::pair<uint, uint> > buys;

	virtual void RunQuery()
	{
		all_trades.GetData(DEBUGARG);
		vector<uint> tbuys;
		for (uint u = 0; u < all_trades.users_events.size();u++) {
			auto &events = all_trades.users_events[u];
			for (uint i = all_trades.last_resets[u]; i < events.size();i++) {
				auto &e = events[i];
				if (e.player_id >= tbuys.size()) tbuys.resize(e.player_id + 64);
				tbuys[e.player_id]++;
			}
		}
		for (uint i = 0; i < tbuys.size(); i++) {
			if (tbuys[i]>0) buys.push_back(std::make_pair(i, tbuys[i]));
		}
		sort(buys.begin(), buys.end(), [](const std::pair<uint, uint> &a, const std::pair<uint, uint> &b){
			return a.second > b.second;
		});
	}

	string Line(uint rank, uint pid, uint buys)
	{
		string ret;
		ret = "<mdiv>#" + ToString(rank) + " " + players.phtml(pid) + " with " + ToString(buys) + (buys != 1 ? " buys " : " buy ") + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < buys.size() && i<25; i++) {
			if (buys[i].second <= 1) break;
			if (i == 5) ret += showmore.Start("Most Bought");
			ret += Line(i + 1, buys[i].first, buys[i].second);
		}
		ret += showmore.End();
		return ret;
	}
};

class UntappedPotential : public Module
{
public:
	vector<ChangeSpan> spans;
	MostBought *most_bought;

	UntappedPotential(MostBought *mb) {
		most_bought = mb;
	}

	virtual void RunQuery()
	{
		all_trades.GetData(DEBUGARG);
		players_value_history.GetData(DEBUGARG);
		MostBought &mb = *most_bought;
		mb.GetData(DEBUGARG);
		for (auto &c : players_value_history.change_spans) {
			uint buys = 0;
			for (auto &b : mb.buys) {
				if (b.first == c.pid) buys = b.second;
			}
			if (buys > 0) continue;
			spans.push_back(c);
		}
	}

	string Line(uint rank, ChangeSpan span)
	{
		string ret;
		uint start_time = players_value_history.timestamps[span.start_slot];
		uint end_time = players_value_history.timestamps[span.end_slot];
		uint ispan = end_time - start_time;
		string sspan = ToString(ispan/86400) + " days";
		if (ispan < 86400 * 3) sspan = ToString(ispan / 3600) + " hours";
		ret = "<mdiv>#" + ToString(rank) + " " + players.phtml(span.pid) + " gained " + ToString(span.end_val - span.start_val) + " points over " + sspan;
		ret += ", going from " + ToString(span.start_val) + " points on " + TimestampHtml(start_time) + ", to " + ToString(span.end_val) + " points on " + TimestampHtml(end_time) + "." + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for (uint i = 0; i < spans.size() && i<25; i++) {
			if (i == 5) ret += showmore.Start("Untapped Potential");
			ret += Line(i + 1, spans[i]);
		}
		ret += showmore.End();
		return ret;
	}
};

class JudgeTiming : public Module
{
public:
	vector< std::pair<std::pair<TeamHistoryEvent, TeamHistoryEvent>,ChangeSpan> > trades;
	//vector<ChangeSpan> ideal_spans;//match the timestamps to the trade timing if it's in the same range, to make it easy to check if it was ideal

	static int score(const std::pair<std::pair<TeamHistoryEvent, TeamHistoryEvent>, ChangeSpan> &a)
	{
		int val_a = ((int)a.first.second.amount - (int)a.first.first.amount) - (a.second.end_val - a.second.start_val);
		if (a.first.second.amount>a.first.first.amount) val_a += 10000;
		if (a.second.end_val < a.second.start_val) {
			val_a = ((int)a.first.second.amount - (int)a.first.first.amount) + (a.second.end_val - a.second.start_val);
		}
		val_a += ((int)a.first.second.amount - (int)a.first.first.amount);
		return val_a;
	}

	virtual void RunQuery()
	{
		gamesimheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		all_trades.GetData(DEBUGARG);
		players_value_history.GetData(DEBUGARG);
		best_worst_trades.GetData(DEBUGARG);
		uint days_scan = 14;
		for (auto &t : best_worst_trades.events) {
			ChangeSpan ideal;
			ideal.pid = t.first.player_id;
			ChangeSpan worst = ideal;
			worst.start_slot = t.first.timestamp;
			worst.end_slot = t.second.timestamp;
			worst.start_val = (int)t.first.amount;
			worst.end_val = (int)t.second.amount;
			//int gain = worst.end_val - worst.start_val;
			int worst_gain = 0;
			int ideal_gain = 0;

			for (auto &c : players_value_history.change_spans_full) {
				if (c.pid != t.first.player_id) continue;
				ChangeSpan tspan = ideal;
				tspan.start_slot = players_value_history.timestamps[c.start_slot];
				tspan.end_slot = players_value_history.timestamps[c.end_slot];
				tspan.start_val = c.start_val;

				uint next_start = t.first.timestamp+1;
				if (players_value_history.timestamps.size() > c.start_slot+1) next_start = players_value_history.timestamps[c.start_slot + 1];
				uint next_end = t.second.timestamp+1;
				if (players_value_history.timestamps.size() > c.end_slot+1) next_end = players_value_history.timestamps[c.end_slot + 1];

				if (t.first.timestamp > tspan.start_slot && t.first.timestamp < next_start) tspan.start_slot = t.first.timestamp;
				else if (tspan.start_slot < t.first.timestamp) tspan.start_slot = tspan.start_slot/2 + next_start/2;
				else tspan.start_slot = tspan.start_slot / 2 + next_start / 2;
				if (t.second.timestamp > tspan.end_slot && t.second.timestamp < next_end) tspan.end_slot = t.second.timestamp;
				else if (tspan.end_slot < t.second.timestamp) tspan.end_slot = tspan.end_slot/2 + next_end/2;
				else tspan.end_slot = tspan.end_slot / 2 + next_end / 2;

				//if (t.first.timestamp - 15 * 86400 > tspan.end_slot || t.second.timestamp + 15 * 86400 < tspan.start_slot) continue;
				if (t.first.timestamp - days_scan * 86400 > tspan.start_slot || t.first.timestamp + days_scan * 86400 < tspan.start_slot) continue;
				if (t.second.timestamp - days_scan * 86400 > tspan.end_slot || t.second.timestamp + days_scan * 86400 < tspan.end_slot) continue;

				if (tspan.end_slot<wwglobals.season_end-86400-4000) tspan.end_val = c.end_val*wwglobals.sell_percent / 100;
				else tspan.end_val = c.end_val;

				int tgain = tspan.end_val - tspan.start_val;
				if (tgain > ideal_gain) {
					ideal = tspan;
					ideal_gain = tgain;
				}
				else if (tgain < worst_gain) {
					worst = tspan;
					worst_gain = tgain;
				}
			}

			if (ideal_gain>0) {
				trades.push_back( std::make_pair(t, ideal) );
			}
			if (worst_gain < 0) {
				trades.push_back( std::make_pair(t, worst) );
			}
		}

		sort(trades.begin(), trades.end(), [this](const std::pair<std::pair<TeamHistoryEvent, TeamHistoryEvent>, ChangeSpan> &a, const std::pair<std::pair<TeamHistoryEvent, TeamHistoryEvent>, ChangeSpan> &b) {
			int val_a = score(a);
			int val_b = score(b);
			return val_a > val_b;
		});
	}

	string Line(uint rank, std::pair<std::pair<TeamHistoryEvent, TeamHistoryEvent>, ChangeSpan> t)
	{
		string ret;
		User u;
		u.user_id = t.first.first.uid;
		u.username = all_trades.usernames[u.user_id];
		u.isloaded = true;
		int gain = (int)t.first.second.amount - (int)t.first.first.amount;
		string schange = " gained " + ToString(gain)+" points";
		if (gain < 0) schange = " lost " + ToString(gain*-1) + " points";
		int comp_gain = t.second.end_val - t.second.start_val;
		ret = "<mdiv>#" + ToString(rank) + " " + u.namehtml() + " with " + players.phtml(t.first.first.player_id);
		ret += schange;
		int buy_off = (int)(t.second.start_slot - 1000000000) - (int)(t.first.first.timestamp - 1000000000);
		int sell_off = (int)(t.second.end_slot - 1000000000) - (int)(t.first.second.timestamp - 1000000000);
		int buy_days_off = buy_off/86400;
		int sell_days_off = sell_off/86400;
		bool perfect = false;
		if ((double)comp_gain / (double)gain > 0.9 && (double)comp_gain / (double)gain < 1.1 && buy_off==0 && sell_off==0) {
			ret += " and was perfectly timed";
			perfect = true;
		}
		else if (comp_gain>0) {
			ret += string()+" but could've "+(gain==comp_gain?"still ":"")+"gained "+ToString(comp_gain)+" points";
		}
		else if (comp_gain<0) {
			ret += string() + " but could've " + (gain == comp_gain ? "still " : "") + "lost " + ToString(comp_gain*-1) + " points";
		}
		else {
			ret += "wtf...";
		}
		if (perfect==false) {
			//ret += " if bought ";
			if (std::abs(buy_off)>=86400 * 2) {
				ret += " if bought " + ToString(std::abs(buy_days_off)) + " days " + (buy_off > 0 ? "later" : "earlier");
			}
			else if(buy_off!=0) {
				ret += " if bought " + ToString(std::abs(buy_off/3600)) + " hours " + (buy_off > 0 ? "later" : "earlier");
			}
			else {
				ret += " if";
			}
			if (buy_off != 0 && sell_off != 0) ret += " and";
			if (std::abs(sell_off) >= 86400 * 2) {
				ret += " sold " + ToString(std::abs(sell_days_off)) + " days " + (sell_off > 0 ? "later" : "earlier");
			}
			else if(sell_off!=0) {
				ret += " sold " + ToString(std::abs(sell_off/3600)) + " hours " + (sell_off > 0 ? "later" : "earlier");
			}
		}
		ret += string(".") + (globalcontext.r == Renderer::bbcode ? "<br/><br/>" : "") + "</mdiv>";
		return ret;
	}

	string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		uint rank = 1;
		for (auto &t : trades) {
			if (t.second.end_val < t.second.start_val) continue;
			if (rank == 11) ret += showmore.Start("Best Timinigs");
			ret += Line(rank, t);
			rank++;
		}
		for (auto &t : trades) {
			if (t.second.end_val > t.second.start_val) continue;
			if (rank == 11) ret += showmore.Start("Best Timinigs");
			ret += Line(rank, t);
			rank++;
		}
		ret += showmore.End();
		return ret;
	}

	string BestTiming(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		uint rank = 1;
		for (auto &t : trades) {
			if (t.second.end_val < t.second.start_val) continue;
			if (rank == 6) ret += showmore.Start("Best Timinigs");
			ret += Line(rank, t);
			rank++;
		}
		ret += showmore.End();
		return ret;
	}

	string WorstTiming(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ShowMore showmore;
		uint rank = 1;
		for (uint i = (uint)trades.size() - 1; i<trades.size();i--) {
			auto &t = trades[i];
			if (t.second.end_val > t.second.start_val) continue;
			if (rank == 6) ret += showmore.Start("Worst Timinigs");
			ret += Line(rank, t);
			rank++;
		}
		ret += showmore.End();
		return ret;
	}
};

string CheckTradeLock()
{
	gamesimheader.GetData(DEBUGARG);
	wwglobals.GetData(DEBUGARG);
	players.GetData(DEBUGARG);
	headbands.GetData(DEBUGARG);
	current_user.GetData(DEBUGARG);
	current_user_team.QueryForUpdate();
//	return string();

	if (current_user.user_id == 0) {
		return string("{\"success\":\"0\",\"error\":\"You need to be logged in to do this\"}");
	}
	if (current_user.user_id == 2) {
		return string("{\"success\":\"0\",\"error\":\"No, bad Justin!\"}");
	}
	if (wwglobals.force_trade_lock) {
		return string("{\"success\":\"0\",\"error\":\"Trading is currently locked\"}");
	}
	if (simheader.samples<wwglobals.min_samples) {
		return string("{\"success\":\"0\",\"error\":\"You cannot do this while the current simulation has less than " + ToStringCommas(wwglobals.min_samples) + " samples\"}");
	}
	uint mysearches = 0;
	matchsearches.GetData(DEBUGARG);
	for (auto m : matchsearches.match_searches) {
		if (m.made == 0 && m.user_id == current_user.user_id) {
			mysearches++;
		}
	}
	if (mysearches>0) {
		//return string("{\"success\":\"0\",\"error\":\"You cannot trade players while searching for a match\"}");
	}
	upcoming.GetData(DEBUGARG);
	upcomingmatches.GetData(DEBUGARG);
	for (auto &upm : upcomingmatches.matches) {
		if (upm.timestamp == 0) continue;
		if (upm.timestamp<(uint)time(0)) {
			return string("{\"success\":\"0\",\"error\":\"You cannot trade players while an SC2 match is in progress\"}");
		}
	}
	upcomingtournaments.GetData(DEBUGARG);
	tournaments.GetData(DEBUGARG);
	for (auto &t : upcomingtournaments.tournies) {
		if (t.timestamp == 0) continue;
		//if (t.evgs1.size() == 0) continue;
		if (t.timestamp<(uint)time(0)) {
			//DEBUGOUT(ERR, t.t_id, "cannot trade players while tournament going on - " << t.timestamp << ", " << (uint)time(0) << ", " << t.evgs1.size());
			return string("{\"success\":\"0\",\"error\":\"You cannot trade players while an SC2 tournament (" + jsonescape(tournaments.tournies[t.t_id].name, 256) + ") is in progress\"}");
		}
	}
	return string();
}

string BuyPlayer(uint id,uint cost)
{
	string ret = CheckTradeLock();
	DEBUGOUT(WARN, current_user.user_id, current_user.username << ", BuyPlayer, id== " << id<<", cost=="<<cost<<", CheckTradeLock ret=="<<ret<<"\n");
	if (ret.length()) return ret;
	
	if(id>players.players.size()) {
		return string("{\"success\":\"0\",\"error\":\"Player ID out of bounds\"}");
	} else if(players.players[id].player_id!=id) {
		return string("{\"success\":\"0\",\"error\":\"Player ID is invalid\"}");
	}
	auto &p=players.players[id];
	for(auto tp : current_user_team.team) {
		if(p.player_id==tp.player_id) {
			return string("{\"success\":\"0\",\"error\":\"You already have this player on your team\"}");
		}
	}
	uint wcsps=PlayerValue(id);
	if(cost!=wcsps) {
		return string("{\"success\":\"0\",\"error\":\"Player Cost is invalid\"}");
	}
	if(cost>current_user.wcspoints) {
		return string("{\"success\":\"0\",\"error\":\"You don't have enough WCS Points\"}");
	}
	if(current_user_team.team.size()>=wwglobals.max_players) {
		return string("{\"success\":\"0\",\"error\":\"Your team is full\"}");
	}
	mysqlcon.update(("update users set wcs_points=wcs_points-"+ToString(cost)+" where user_id="+ToString(current_user.user_id)).c_str() DEBUGARGS);
	mysqlcon.insert(("insert into users_players set user_id="+ToString(current_user.user_id)+", player_id="+ToString(id)+", bought_for="+ToString(cost)+", active=1, bought_on=now()").c_str() DEBUGARGS);
	ret="{\"success\":\"1\"}";
	return ret;
}

string SellPlayer(uint id,uint cost)
{
	string ret = CheckTradeLock();
	DEBUGOUT(WARN, current_user.user_id, current_user.username << ", SellPlayer, id== " << id << ", cost==" << cost << ", CheckTradeLock ret==" << ret<<"\n");
	if (ret.length()) return ret;
	
	if(id>players.players.size()) {
		return string("{\"success\":\"0\",\"error\":\"Player ID out of bounds\"}");
	} else if(players.players[id].player_id!=id) {
		return string("{\"success\":\"0\",\"error\":\"Player ID is invalid\"}");
	}
	auto &p=players.players[id];
	bool haveplayer=false;
	for(auto tp : current_user_team.team) {
		if(p.player_id==tp.player_id) {
			haveplayer=true;
			break;
		}
	}
	if(!haveplayer) return string("{\"success\":\"0\",\"error\":\"You don't have this player on your team\"}");
	
	uint wcsps=PlayerSellValue(id);
	if(cost!=wcsps) {
		return string("{\"success\":\"0\",\"error\":\"Player Cost is invalid\"}");
	}
	mysqlcon.update(("update users set wcs_points=wcs_points+"+ToString(cost)+" where user_id="+ToString(current_user.user_id)).c_str() DEBUGARGS);
	mysqlcon.update(("update users_players set active=0, sold_for="+ToString(cost)+", sold_on=now() where active=1 and user_id="+ToString(current_user.user_id)+" and player_id="+ToString(id)).c_str() DEBUGARGS);
	ret="{\"success\":\"1\"}";
	return ret;
}

string ResetPlayers()
{
	string ret = CheckTradeLock();
	DEBUGOUT(WARN, current_user.user_id, current_user.username << ", ResetPlayers, CheckTradeLock ret==" << ret<<"\n");
	if (ret.length()) return ret;
	
	mysqlcon.update(("update users set wcs_points="+ToString(wwglobals.starting_wcs_points)+" where user_id="+ToString(current_user.user_id)).c_str() DEBUGARGS);
	mysqlcon.update(("update users_players set active=0, sold_for=0, sold_on=now() where active=1 and user_id="+ToString(current_user.user_id)).c_str() DEBUGARGS);
	
	ret="{\"success\":\"1\"}";
	return ret;
}

string StartFindMatch(string format)
{
	string ret;
	current_user.GetData(DEBUGARG);
	if(current_user.user_id==0) {
		return string("{\"success\":\"0\",\"error\":\"You need to be logged in to do this\"}");
	}
	format=ToLower(format);
	if(format=="1v1") {
		format="1v1";
	} else if(format=="2v2") {
		format="2v2";
	} else if(format=="ffa") {
		format="ffa";
	} else if(format=="passion") {
		return string("{\"success\":\"0\",\"error\":\"Passion not yet available\"}");
	} else if(format=="coop") {
		return string("{\"success\":\"0\",\"error\":\"Co-op play is not yet available\"}");
	} else {
		return string("{\"success\":\"0\",\"error\":\"Format is invalid\"}");
	}
	matchsearches.GetData(DEBUGARG);
	for(auto m : matchsearches.match_searches) {
		if(m.user_id==current_user.user_id && m.format==format) {
			return string("{\"success\":\"0\",\"error\":\"Cannot search for multiple matches of the same format\"}");
		}//maybe I should restrict to 2 searches per format? or 5 searches overall??
	}
	currentmatches.GetData(DEBUGARG);
	if(currentmatches.matches.size()>20) {
		return string("{\"success\":\"0\",\"error\":\"Cannot have so many current matches\"}");
	}
	wwglobals.GetData(DEBUGARG);
	current_user_team.GetData(DEBUGARG);
	if(current_user_team.team.size()<wwglobals.min_players) {
		return ("{\"success\":\"0\",\"error\":\"You must have at least "+ToString(wwglobals.min_players)+" players on your team\"}");
	}
	if(current_user_team.team.size()>wwglobals.max_players) {
		return ("{\"success\":\"0\",\"error\":\"You must have no more than "+ToString(wwglobals.max_players)+" players on your team\"}");
	}
	mysqlcon.insert(("insert into match_searches set user_id="+ToString(current_user.user_id)+",created=now(),format='"+mysqlcon.escape(format,32)+"',mmr="+ToString(current_user.mmr)+",mmrspread=100").c_str() DEBUGARGS);
	matchsearches.match_searches.clear();
	matchsearches.isloaded=false;
	matchsearches.GetData(DEBUGARG);
	ret="{\"success\":\"1\"}";
	return ret;
}
