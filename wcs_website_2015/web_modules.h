

class Module {
public:
	bool isloaded;
	//const Context c;

	Module()// : c(globalcontext)
	{
		isloaded = false;
	}

	/*Module(const Context context) : c(context)
	{
		isloaded = false;
		//c=context;
	}*/

	virtual ~Module()
	{
	}

	virtual void RunQuery() {}
	virtual void GetData(DEBUGARGDEF) { if (isloaded) return; RunQuery(); isloaded = true; }
	virtual string InnerHtml() = 0;
	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return InnerHtml(); }
	virtual string BBCode() { return Html(DEBUGARG); }
	//virtual string shortHtml() { return Html(DEBUGARG); }
};

class ModuleSandwich : public Module {//do I need this?
public:
	string pre;
	string post;
	virtual string HtmlPrefix() { return pre; }
	virtual string HtmlPostfix() { return post; }
	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return HtmlPrefix() + InnerHtml() + HtmlPostfix(); }
};

class User : public Module {
public:
	uint user_id;
	uint wcspoints;
	uint mmr;
	string username;
	string tlname;
	string redditname;
	string twitchname;
	string twittername;
	//vector<UserPlayer> team;
	//uint teamvalue;

	User() {
		user_id = 0;
		wcspoints = 0;
		mmr = 100;
		//teamvalue = 0;
	}

	void Init(uint UserID, string UserName, uint WCSPoints, string TL, string Reddit, string Twitch, string Twitter)
	{
		username = UserName;
		user_id = UserID;
		wcspoints = WCSPoints;
		tlname = TL;
		redditname = Reddit;
		twitchname = Twitch;
		twittername = Twitter;

		/*players.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);
		auto *res = mysqlcon.select(("select player_id,bought_for,sold_for,active,unix_timestamp(bought_on),unix_timestamp(sold_on) from users_players where active=1 and user_id=" + ToString(user_id) + " order by player_id").c_str() DEBUGARGS);

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
			if (up.player_id<players.players.size() && up.player_id == players.players[up.player_id].aligulac_id) {
				up.headband = players.players[up.player_id].headband;
			}
			team.push_back(up);
		}

		CalcTeamValue();*/
		isloaded = true;
	}

	virtual void RunQuery()
	{
		auto * res = mysqlcon.select(("select username, wcs_points, tlname, redditname, twitchname, twittername from users where user_id=" + ToString(user_id)).c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			//uint p_id = rstring(row[0]);
			username = row[0].cstr();
			wcspoints = row[1];
			tlname = row[2].cstr();
			redditname = row[3].cstr();
			twitchname = row[4].cstr();
			twittername = row[5].cstr();
		}
		//Init(user_id, username, wcspoints);
		isloaded = true;
	}


	void Save()
	{
		mysqlcon.update(("update users set tlname='"+mysqlcon.escape(tlname, 120)+"', redditname='"+mysqlcon.escape(redditname, 120)+"', twitchname='"+mysqlcon.escape(twitchname,120)+"', twittername='"+mysqlcon.escape(twittername,120)+"' where user_id=" + ToString(user_id)).c_str() DEBUGARGS);
	}

	/*uint CalcTeamValue()
	{
		teamvalue = 0;
		for (auto &p : team) {
			teamvalue += PlayerValue(p.player_id);
		}
		return teamvalue;
	}*/

	string InnerHtml() {
		/*string teamjson;
		for (auto p : team) {
			teamjson += "{id:" + ToString(p.player_id) + ",headband:" + ToString(p.headband) + ",active:" + ToString(p.active) + "},";
		}
		if (teamjson.length()) teamjson = teamjson.substr(0, teamjson.length() - 1);
		return "<script>window.wcs.currentuser={id:" + ToString(user_id) + ",username:'" + cono.MysqlEscape(username, 256) + "',wcspoints:" + ToString(wcspoints) + ",team:[" + teamjson + "]};</script>";*/
		return "<script>window.wcs.currentuser={id:" + ToString(user_id) + ",username:'" + mysqlcon.escape(username, 256) + "',wcspoints:" + ToString(wcspoints) + "};</script>";
	}

	string namehtml()
	{
		string trophies;
		if (user_id == 19) {
			trophies += "<img src='/img/trophy-icon.png' style='margin:-3px 3px;' title='Season 1 Champion'>";
			if (globalcontext.r == Renderer::bbcode) trophies += "[img]http://sc2.4ever.tv/img/trophy-icon.png[/img] ";
		}
		else if (user_id == 42) {
			trophies += "<img src='/img/trophy-icon.png' style='margin:-3px 3px;' title='Season 3 Champion'><img src='/img/trophy-silver-icon.png' style='margin:-3px 3px;' title='Season 1 Runner-Up'>";
			if (globalcontext.r == Renderer::bbcode) trophies += "[img]http://sc2.4ever.tv/img/trophy-icon.png[/img][img]http://sc2.4ever.tv/img/trophy-silver-icon.png[/img] ";
		}
		else if (user_id == 29) {
			trophies += "<img src='/img/trophy-icon.png' style='margin:-3px 3px;' title='Season 2 Champion'>";
			if (globalcontext.r == Renderer::bbcode) trophies += "[img]http://sc2.4ever.tv/img/trophy-icon.png[/img] ";
		}
		else if (user_id == 35) {
			trophies += "<img src='/img/trophy-silver-icon.png' style='margin:-3px 3px;' title='Season 2 Runner-Up'>";
			if (globalcontext.r == Renderer::bbcode) trophies += "[img]http://sc2.4ever.tv/img/trophy-silver-icon.png[/img] ";
		}
		string html = "<a href='" + MakeURL("page=user&uid=" + ToString(user_id)) + "' class='user' style='font-weight:bold;'>" + HtmlEscape(username, 256) + "</a>";
		if (globalcontext.r == Renderer::bbcode) {
			return trophies+"[url=http://sc2.4ever.tv" + MakeURL("page=user&uid=" + ToString(user_id)) + "][b]" + html + "[/b][/url]";
		}
		return trophies+html;
	}
};
User current_user;

class HeaderBar : public Module{
public:
	string pagename;
	string homelink;
	string rightlinkurl;
	string rightlinkname;
	uint alerts;//later might do a vector of all the alerts? why if they're going to be on a separate page? or maybe just click to expand?
	//maybe just look up the alerts from the user module, might be able to do it with that same query

	HeaderBar()
	{
		alerts = 0;
	}

	virtual void RunQuery() {}
	virtual string InnerHtml()
	{
		if (rightlinkname.length() == 0 && rightlinkurl.length() == 0) {
			rightlinkurl = "page=gamehome";
			rightlinkname = "<i class='fa fa-play'></i>WCS Wars";
		}
		if (pagename.length() == 0) {
			pagename = "<i class='fa fa-home'></i>WCS Predictor 2015";
		}
		string ret;
		string envelope = "<a href='" + MakeURL("page=messages") + "' title='No new messages or alerts.'><i class='fa fa-envelope-o noalert'></i></a>";
		if (alerts) envelope = "<a href='" + MakeURL("page=messages") + "' title='"+ToString(alerts)+" new alerts/messages.'><i class='fa fa-envelope newalert'></i></a>";
		ret += "<h1 class='header-bar'><a href='" + MakeURL(homelink, true) + "'>" + pagename + "</a>";
		ret += "<span style='float:right;dislay:inline-block;'>";
		ret += "<a href='" + MakeURL(rightlinkurl, true) + "' style='padding-right:1em;'>" + rightlinkname + "</a>";
		//ret += "<ul class='pages-menu'>";
		
		//ret += envelope;
		//ret += "<a href='" + MakeURL("page=settings") + "' title='Settings'><i class='fa fa-cog'></i></a><a href='"+MakeURL("page=faq")+"' title='Help and FAQ'><i class='fa fa-question'></i></a>";
		ret += "<a href='" + MakeURL("page=faq") + "' title='Help and FAQ'><i class='fa fa-question-circle'></i></a>";

		//ret += "<ul class='pages-menu'><li><a class='pages-arrow' href='"+MakeURL("page=pages")+"'><i class='fa fa-angle-down'></i></a><ul><li>test page 1</li><li>test page 2</li><li>test page 3</li></ul></li></ul>";
		//ret += "<ul class='pages-menu'><li><i class='fa fa-angle-down'></i><ul>";
		ret += "<ul class='pages-menu'>";
		ret += "<li><a href='"+MakeURL("page=pages")+"'><i class='fa fa-angle-down' style='font-weight:bold;'></i>Menu</a><ul>";

		ret +=
			"<li><a href='" + MakeURL("", true) + "'>Home</a></li>"
			"<li><a href='" + MakeURL("page=gamehome", true) + "'>WCS Wars</a></li>"
			"<li><a href='" + MakeURL("page=faq", true) + "'>Help and FAQ</a></li>"
			"<li><a href='" + MakeURL("page=players") + "'>Full Player List</a></li>"
			"<li><a href='" + MakeURL("page=checkup", true) + "'>Checkup</a></li>"
			"<li><a href='" + MakeURL("page=must_wins") + "'>Must Win Players</a></li>"
			"<li><a href='" + MakeURL("page=tournaments") + "'>Tournaments</a></li>"
			"<li><a href='" + MakeURL("page=countries") + "'>Countries</a></li>"
			"<li><a href='" + MakeURL("page=simulations", true) + "'>Simulations</a></li>"
			"<li><a href='" + MakeURL("page=branches", true) + "'>Branches</a></li>"
			"<li><a href='" + MakeURL("page=login", true) + "'>Profile</a></li>"
			"<li><a href='http://www.teamliquid.net/forum/starcraft-2/471444-wcs-predictor-2015'>Discussion Thread</a></li>";

		ret += "</ul></li></ul>";

		ret += "</span>";
		ret += "</h1>\n";
		return ret;
	}
};
HeaderBar headerbar;

class UpdateEvent {
public:
	uint sim_id;
	uint created;
	string screated;
	string description;
	uint samps;
	int type;
	
	UpdateEvent(uint SIM_ID, uint CREATED, uint SAMPLES, string SCREATED, string DESCRIPTION, int TYPE) : screated(SCREATED), description(DESCRIPTION)
	{
		sim_id=SIM_ID;
		created=CREATED;
		samps=SAMPLES;
		type = TYPE;
	}
};

class SimHeader : public Module{
public:
	uint sim_id;
	uint samples;
	uint created;
	uint includes_full_year;
	string screated;
	
	uint comp_sim_id;
	uint comp_samples;
	uint comp_created;
	string comp_screated;

	uint type;
	uint base_sim_id;
	string branch_desc;

	//string sim_notes;
	//string comp_sim_notes;
	
	vector<UpdateEvent> update_events;
	
	SimHeader()
	{
		sim_id=0;
		samples=0;
		created=0;
		includes_full_year=0;
		
		comp_sim_id=0;
		comp_samples=0;
		comp_created=0;

		//sim_notes="Notes for this simulation go here.";
		//comp_sim_notes="Notes for the comparison simulation go here.";
		type = 1;
		base_sim_id = 0;
	}
	
	virtual void RunQuery()
	{
		string query = "select sim_id,size,created, unix_timestamp(created),includes_full_year,type,base_sim_id,description from sims left join branches using(sim_id) where size>=50000 and use_aligulac="+ToString(USE_ALIGULAC)+" and sims.type="+ToString(SIM_TYPE)+" order by created desc limit 1";
		if ((uint)vars["sim_id"] > 0) {
			query = "select sim_id,size,created, unix_timestamp(created),includes_full_year,type,base_sim_id,description from sims left join branches using(sim_id) where sim_id=" + ToString((uint)vars["sim_id"]);
		}
		auto * res = mysqlcon.select(query.c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		sim_id = 0;
		samples = 0;
		created = 0;
		includes_full_year = 0;
		type = 1;
		base_sim_id = 0;
		branch_desc = string();

		if (num_rows) {
			auto row = mysqlcon.fetch_row(res);
			sim_id = row[0];
			samples = row[1];
			
			screated = row[2].cstr();
			created = row[3];
			includes_full_year = row[4];
			type = row[5];
			base_sim_id = row[6];
			branch_desc = row[7].ToString();
		}
		
		//if (base_sim_id && (!vars["comp_sim_id"]) && (!vars["comp_days"]) && (!vars["comp_hours"]) ) vars["comp_sim_id"] = base_sim_id;

		comp_sim_id = (uint)vars["comp_sim_id"];
		uint comp_days = (uint)vars["comp_days"];
		uint comp_hours = (uint)vars["comp_hours"];
		if (comp_sim_id == 0 && !vars["comp_days"] && !vars["comp_hours"]) {
			if (base_sim_id) comp_sim_id = base_sim_id;
			//else comp_days = 5;
		}
		if(comp_sim_id) {
			query = "select sim_id,size,created, unix_timestamp(created),includes_full_year from sims where sim_id="+ToString(comp_sim_id)+" and use_aligulac="+ToString(USE_ALIGULAC)+" order by created desc limit 1";
		}
		else if (vars["comp_days"]) {
			query = "select sim_id,size,created, unix_timestamp(created),includes_full_year from sims where created<'" + mysqlcon.escape(screated, 256) + "'-interval "+ToString(comp_days)+" day and size>=" + ToString(HISTORY_MIN_SIZE) + " and use_aligulac=" + ToString(USE_ALIGULAC) + " and sims.type="+ToString(SIM_TYPE)+" order by created desc limit 1";
		}
		else if (vars["comp_hours"]) {
			query = "select sim_id,size,created, unix_timestamp(created),includes_full_year from sims where created<'" + mysqlcon.escape(screated, 256) + "'-interval "+ToString(comp_hours)+" hour and size>=" + ToString(HISTORY_MIN_SIZE) + " and use_aligulac=" + ToString(USE_ALIGULAC) + " and sims.type="+ToString(SIM_TYPE)+" order by created desc limit 1";
		} else {
			query = "select sim_id,size,created, unix_timestamp(created),includes_full_year from sims where created<'" + mysqlcon.escape(screated, 256) + "'-interval 5 day and size>=" + ToString(HISTORY_MIN_SIZE) + " and use_aligulac=" + ToString(USE_ALIGULAC) + " and sims.type="+ToString(SIM_TYPE)+" order by created desc limit 1";
		}
		res = mysqlcon.select(query.c_str() DEBUGARGS);
		num_rows = mysql_num_rows(res);
		comp_sim_id = 0;
		comp_samples = 0;
		comp_created = 0;
		
		if (num_rows) {
			auto row = mysqlcon.fetch_row(res);
			comp_sim_id = row[0];
			comp_samples = row[1];
			
			comp_screated = row[2].cstr();
			comp_created = row[3];
		}

		//query = "select * from (select created,unix_timestamp(created),sim_id,description,size,type from sims left join update_events using(created) where created between from_unixtime(" + ToString(min(comp_created, created - HISTORY_DAYS * 86400)) + ") and from_unixtime(" + ToString(created) + ") union select created,unix_timestamp(created),sim_id,description,size,type from sims right join update_events using(created) where created between from_unixtime(" + ToString(min(comp_created, created - HISTORY_DAYS * 86400)) + ") and from_unixtime(" + ToString(created) + ") and sim_id is null) as t1 order by created asc";
		if (comp_created < created - HISTORY_DAYS * 86400) {
			HISTORY_DAYS = (created - comp_created + 86400) / 86400;
		}
		query = "select * from (select created,unix_timestamp(created),sim_id,description,size,type from sims left join update_events using(created) where created between from_unixtime(" + ToString(min(comp_created, created - HISTORY_DAYS * 86400)-86400*4) + ") and from_unixtime(" + ToString(created) + ") union select created,unix_timestamp(created),sim_id,description,size,type from sims right join update_events using(created) where created between from_unixtime(" + ToString(min(comp_created, created - HISTORY_DAYS * 86400)-86400*4) + ") and from_unixtime(" + ToString(created) + ") and sim_id is null) as t1 order by created asc";
		res = mysqlcon.select(query.c_str() DEBUGARGS);
		num_rows = mysql_num_rows(res);
		update_events.clear();
		for(decltype(num_rows) r=0;r<num_rows;r++) {
			auto row=mysqlcon.fetch_row(res);
			string sc=row[0].ToString();
			uint c=row[1];
			uint sid=row[2];
			string desc=row[3].ToString();
			uint samps=row[4];
			int type = row[5];
			if (type != 1 && sid!=sim_id && sid!=comp_sim_id) sid = 0;
			if (sid == sim_id && branch_desc.length()) desc = branch_desc;
			update_events.push_back( UpdateEvent(sid, c, samps, sc, desc, type) );
		}
	}
	
	string InnerHtml()
	{
		if (globalcontext.r == Renderer::plain) {
			string ret = "<a href='"+MakeURL("sim_id="+ToString(sim_id),true)+"'>Simulation #" + ToString(sim_id) + "</a>, using results from "+screated+", has " + ToStringCommas(samples) + " samples.\n";
			ret += "-Comparing to <a href='"+MakeURL("sim_id="+ToString(comp_sim_id),true)+"'>Simulation #" + ToString(comp_sim_id) + "</a>, using results from " + comp_screated + ", has " + ToStringCommas(comp_samples) + " samples.\n";
			return ret;
		}
		string ret;
		ret+="<script>window.wcs.sim={sim_id:"+ToString(sim_id)+",created:"+ToString(created)+",samples:"+ToString(samples)+"};window.wcs.top_num_qualify="+ToString(TOP_NUM_QUALIFY)+";";
		ret+="window.wcs.comp_sim={sim_id:"+ToString(comp_sim_id)+",created:"+ToString(comp_created)+",samples:"+ToString(comp_samples)+"};window.wcs.top_num_qualify="+ToString(TOP_NUM_QUALIFY)+";";
		ret+="</script>\n";
		if(USE_ALIGULAC==1) {
			ret += "<mdiv>Using Aligulac ratings</mdiv>";
		} else {
			ret += "<mdiv>All players as equal</mdiv>";
		}
		/*ret+=SubSection("Simulation #"+ToString(sim_id)+", using results from "+TimestampHtml(created)+", and has "+ToStringCommas(samples)+" samples.", sim_notes);
		ret +=SubSection(" - Comparing to Simulation #"+ToString(comp_sim_id)+", using results from "+TimestampHtml(comp_created)+", and has "+ToStringCommas(comp_samples)+" samples.", comp_sim_notes);
		ret += "<div>Maybe list some matches that happened between them here? Show all the update_events, maybe do like CURRENT SIM #, samples, update_event, then show update_events between them, then show the COMP SIM #, samples, update_event?</div>";*/
		ret+="<mdiv><a href='"+MakeURL("sim_id="+ToString(sim_id),true)+"'>Simulation #"+ToString(sim_id)+"</a>, using results from "+TimestampHtml(created)+", and has "+ToStringCommas(samples)+" samples.</mdiv>";
		ret+="<mdiv> - Comparing to <a href='"+MakeURL("sim_id="+ToString(comp_sim_id),true)+"'>Simulation #"+ToString(comp_sim_id)+"</a>, using results from "+TimestampHtml(comp_created)+", and has "+ToStringCommas(comp_samples)+" samples.</mdiv>";
		string s_update_events;
		string js_update_events;
		for(auto &ue : update_events) {
			uint tsim_id=ue.sim_id;
			if(ue.samps<HISTORY_MIN_SIZE && tsim_id!=sim_id && tsim_id!=comp_sim_id) tsim_id=0;
			if(tsim_id==0 && ue.description.length()==0) continue;

			if (tsim_id>0 && ue.description.length()>0)
				js_update_events += "{c:"+ToString(ue.created)+",i:"+ToString(ue.sim_id)+",s:"+ToString(ue.samps)+",d:'"+jsonescape(ue.description,4096)+"'},";
			else if (ue.description.length()>0)
				js_update_events += "{c:"+ToString(ue.created)+",d:'"+jsonescape(ue.description,4096)+"'},";
			else
				js_update_events += "{c:"+ToString(ue.created)+",i:"+ToString(ue.sim_id)+",s:"+ToString(ue.samps)+"},";

			if (ue.created < comp_created) continue;
			if(tsim_id>0 && ue.description.length()>0)
				s_update_events += "<mdiv>"+TimestampHtml(ue.created)+" - <a href='"+MakeURL("sim_id="+ToString(ue.sim_id),true)+"'>Simulation #"+ToString(ue.sim_id)+"</a> ("+ToStringCommas(ue.samps)+" samples) - "+ue.description+"</mdiv>";
			else if(ue.description.length()>0)
				s_update_events += "<mdiv>"+TimestampHtml(ue.created)+" - "+ue.description+"</mdiv>";
			else
				s_update_events += "<mdiv>"+TimestampHtml(ue.created)+" - <a href='"+MakeURL("sim_id="+ToString(ue.sim_id),true)+"'>Simulation #"+ToString(ue.sim_id)+"</a> ("+ToStringCommas(ue.samps)+" samples)</mdiv>";
		}
		if (js_update_events.length() > 1) {
			js_update_events = js_update_events.substr(0, js_update_events.length() - 1);
			ret += "\n<script>window.wcs.update_events=["+js_update_events+"];</script>\n";
		}
		ret+=SubSection("Simulations Timeline", s_update_events/*+"<mdiv>Maybe also show the matches played during this time period?</mdiv>"*/);
		return ret;
	}
	
	string AgeString()
	{
		return TimestampHtml(created);
		/*string age_string;
		uint age_seconds=(uint)time(0)-created;
		if(age_seconds<120) age_string=ToString(age_seconds)+" seconds old";
		else if(age_seconds/60<120) age_string=ToString((age_seconds+30)/60)+" minutes old";
		else if(age_seconds/3600<48) age_string=ToString((age_seconds+1800)/3600)+" hours old";
		else if(age_seconds/86400<60) age_string=ToString((age_seconds+3600*12)/86400)+" days old";
		else age_string=ToString((age_seconds+86400*15)/(86400*30))+" months old";
		return age_string;*/
	}
};
SimHeader simheader;
//use an alternate GameSimHeader to avoid using aligulac or a specific sim_id or anything like that?
class GameSimHeader : public Module {
public:
	virtual void RunQuery()
	{
		USE_ALIGULAC=1;
		vars["sim_id"]="";
		simheader.isloaded=false;
		simheader.GetData(DEBUGARG);
	}
	
	virtual string InnerHtml()
	{
		return simheader.Html(DEBUGARG);
	}
};
GameSimHeader gamesimheader;

class WWGlobals : public Module {
public:
	uint starting_wcs_points, max_players, min_players, headband1cost, headband2cost, headband3cost, headband4cost, moves_per_turn, moves_per_turn_fast, sell_percent, min_samples;
	int base_damage, base_cost;
	bool force_trade_lock;
	uint season_num,season_start,season_end;

	virtual void RunQuery()
	{
		starting_wcs_points=8000;
		max_players=5;
		min_players=5;
		headband1cost=0;//100;
		headband2cost=0;//50;
		headband3cost=0;//50;
		headband4cost=0;//50;
		moves_per_turn=1;
		moves_per_turn_fast=1;
		sell_percent=98;
		base_damage=1000;
		base_cost=200;
		force_trade_lock=false;//------
		min_samples = 2000000;
		
		season_num=2;//<--------------------
		if ((uint)time(0) >= Timestamp("June 29, 2015 22:00 {{Abbr /CDT")) season_num = 3;
		season_start = 1420348152;//2015-01-03 23:09:12
		season_end = Timestamp("April 6, 2015 22:00 {{Abbr /CDT");
		if (season_num == 2) {
			starting_wcs_points = 12000;
			sell_percent = 99;
			base_cost = 1000;
			season_start = Timestamp("April 6, 2015 22:00 {{Abbr /CDT")-3600*2;
			season_end = Timestamp("June 28, 2015 22:00 {{Abbr /CDT");
		}
		else if (season_num == 3) {
			starting_wcs_points = 18000;
			sell_percent = 99;
			base_cost = 2000;//why did I increase this? to combat inflation I guess?
			season_start = Timestamp("June 29, 2015 22:00 {{Abbr /CDT")-3600*2;
			season_end = Timestamp("October 5, 2015 22:00 {{Abbr /CDT");
		}
		else {//post season?
		}
		/*auto * res = mysqlcon.select("select starting_wcs_points, max_players, min_players, sell_percent, headband1cost, headband2cost, headband3cost, headband4cost, force_trade_lock, moves_per_turn, moves_per_turn_fast, base_damage, base_cost from wwglobals" DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			starting_wcs_points = row[0];
			max_players = row[1];
			min_players = row[2];
			sell_percent = row[3];
			headband1cost = row[4];
			headband2cost = row[5];
			headband3cost = row[6];
			headband4cost = row[7];
			force_trade_lock = ((int)row[8])==1;
			moves_per_turn = row[9];
			moves_per_turn_fast = row[10];
			base_damage = row[11];
			base_cost = row[12];
		}*/
	}

	string InnerHtml() {
		return ("<script>window.wcs.wwglobals={starting_wcs_points:" + ToString(starting_wcs_points) + ",max_players:" + ToString(max_players) + ",min_players:" + ToString(min_players) + ",sell_percent:" + ToString(sell_percent) + ",force_trade_lock:" + ToString((int)force_trade_lock) + ",moves_per_turn:" + ToString(moves_per_turn) + ",moves_per_turn_fast:" + ToString(moves_per_turn_fast) + ",base_damage:" + ToString(base_damage) + ",base_cost:" + ToString(base_cost) + ",headband1cost:" + ToString(headband1cost) + ",headband2cost:" + ToString(headband2cost) + ",headband3cost:" + ToString(headband3cost) + ",headband4cost:" + ToString(headband4cost) + "};</script>");
	}
};
WWGlobals wwglobals;

uint CheckLogin(string username, string password, string session, string create, string newpassword)
{
	if (password.length() == 0) return false;
	string passwordhash = password;// HexMD5(password);
	uint user_id = 0;
	uint wcspoints = 0;
	uint alerts = 0;
	string tl, reddit, twitch, twitter;

	username = URLDecode(username);

	if (create.length() || newpassword.length()) {
		if (username.length() <3 || username.length() >= 32) return 0;
		auto * res = mysqlcon.select(("select user_id, username, wcs_points from users where username='" + mysqlcon.escape(username, 256) + "'").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		if (num_rows > 0) return 0;

		wwglobals.GetData(DEBUGARG);
		user_id = (uint)mysqlcon.insert(("insert into users set username='" + mysqlcon.escape(username, 64) + "', wcs_points=" + ToString(wwglobals.starting_wcs_points) + ", passwordhash='" + mysqlcon.escape(newpassword, 64) + "'").c_str() DEBUGARGS);
		//return user_id;
	}
	auto * res = mysqlcon.select(("select user_id, username, wcs_points, tlname, redditname, twitchname, twittername from users where username='" + mysqlcon.escape(username, 256) + "' and md5(concat(passwordhash,'-so-much-salt'))='" + mysqlcon.escape(passwordhash, 256) + "'").c_str() DEBUGARGS);

	auto num_rows = mysql_num_rows(res);
	for (decltype(num_rows) r = 0; r < num_rows; r++) {
		auto row = mysqlcon.fetch_row(res);
		user_id = row[0];
		username = row[1].cstr();
		wcspoints = row[2];
		tl = row[3].cstr();
		reddit = row[4].cstr();
		twitch = row[5].cstr();
		twitter = row[6].cstr();
	}

	if (user_id) {
		current_user.Init(user_id, username, wcspoints, tl, reddit, twitch, twitter);
		headerbar.alerts = alerts;//I don't put this in the user object because other users won't need this, only the current user needs it
		vars["user_id"] = user_id;
		return user_id;
	}
	return 0;

	/*array<const char*, 10> logins = { {
	"Die4Ever", "caymanr", "Xithryl", "yojustyo", "Tofu33", "killertofu", "Topher", "murica", "patton", "turbo_s", "sisforawesome", "monomachine"
	} };
	for (uint i = 0; i < logins.size(); i += 2) {
	if (username == logins[i] && password == logins[i + 1]) {
	current_user.user_id = i + 1;
	current_user.username = username;
	return i + 1;
	}
	}
	return 0;*/
}

class WCSPointClosest
{
public:
	array<uint,7> points;
	array<uint,7> times;
	array<double,7> closest_to;

	WCSPointClosest()
	{
		for(uint i=0;i<points.size();i++) {
			points[i]=0;
			times[i]=999999999;
			closest_to[i]=50.0;
		}
		closest_to[0]=1.0;
		closest_to[1]=10.0;
		closest_to[2]=30.0;
		closest_to[3]=50.0;
		closest_to[4]=70.0;
		closest_to[5]=90.0;
		closest_to[6]=99.0;
	}

	void count(uint p, uint t, uint samps)
	{
		double dsamps=(double)samps;
		double c=(double)t/dsamps *100.0;
		for(uint i=0;i<points.size();i++) {
			double oc=(double)times[i]/dsamps *100.0;
			double d=std::abs(c-closest_to[i]);
			double od=std::abs(oc-closest_to[i]);
			if( d < od || (i<points.size()/2 && d<=od )  ) {
				points[i]=p;
				times[i]=t;
			}
		}
	}
};

class WCSPCutoffs : public Module
{
public:
	vector< std::pair<uint,uint> > cutoffs;
	WCSPointClosest closests;
	string history;

	virtual void RunQuery()
	{
		WCSPointClosest tclosests;
		//double dsamps=simheader.samples;

		string start=simheader.comp_screated;
		string end=simheader.screated;
		auto *res = mysqlcon.select(("select points, times_top, sim_id, unix_timestamp(sims.created), sims.size from point_cutoffs join sims using(sim_id) where sims.created>'2014-11-01' and sims.created between '" + start + "' and '" + end + "' and (sims.size>=" + ToString(HISTORY_MIN_SIZE) + " or sim_id=" + ToString(simheader.sim_id) + " or sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac=" + ToString(USE_ALIGULAC) + " and (sims.type=" + ToString(SIM_TYPE) + " or sim_id=" + ToString(simheader.sim_id) + " or sim_id="+ToString(simheader.comp_sim_id)+") order by created asc").c_str() DEBUGARGS);
		auto num_rows=mysql_num_rows(res);
		uint last_sim_id=0;
		uint last_created;
		for(decltype(num_rows) r=0;r<num_rows;r++) {
			auto row=mysqlcon.fetch_row(res);
			uint points=row[0];
			uint times_top=row[1];
			uint sim_id=row[2];
			uint created=row[3];
			uint samps=row[4];
			//dsamps=(double)samps;
			if(sim_id==simheader.sim_id) {
				cutoffs.push_back( std::make_pair(points,times_top) );
			}

			if(last_sim_id==0) last_sim_id=sim_id;
			if(last_sim_id!=sim_id) {
				history+="["+ToString(tclosests.points[0])+","+ToString(tclosests.points[3])+","+ToString(tclosests.points[6])+","+ToString(last_created)+"],";
				last_sim_id=sim_id;
				tclosests=WCSPointClosest();
			}
			last_created=created;
			tclosests.count(points, times_top, samps);
		}
		if(last_sim_id) {
			history+="["+ToString(tclosests.points[0])+","+ToString(tclosests.points[3])+","+ToString(tclosests.points[6])+","+ToString(last_created)+"],";
		}
		if(last_sim_id==simheader.sim_id) {
			closests=tclosests;
		}
		if(history.length()) history=history.substr(0,history.length()-1);
	}
	
	string InnerHtml()
	{
		//maybe instead of this complex thing I should just show closest to 10%, closest to 25%, closest to 50%, closest to 75%, closest to 90%?
		//or closest to 10%, 30%, 50%, 70%, 90%?
		uint samps=simheader.samples;
		double dsamps=(double)samps;
		string l;
		double mid=(double)closests.times[3]/dsamps *100.0;

		for(uint i=0;i<closests.points.size();i++) {
			l+="<mdiv class='wcspcutoff'>"+RenderPercent(closests.times[i], samps, mid)+" of the time "+ToString(closests.points[i])+" points is enough to qualify for Blizzcon</mdiv>";
		}
		
		ShowMore showmore;
		l+=showmore.Start("WCS Point Cutoffs");
		for(auto &cutoff : cutoffs) {
			//if(cutoff.first<top_0) continue;
			//if(cutoff.first>bottom_100) continue;
			if(cutoff.second==0 && ((&cutoff)+1)->second==0 ) continue;
			l+="<mdiv class='wcspcutoff'>"+RenderPercent(cutoff.second, samps, mid)+" of the time "+ToString(cutoff.first)+" points is enough to qualify for Blizzcon</mdiv>";
			if(cutoff.second==samps) break;
		}
		l+=showmore.End();
		string r="<div class='graph wcspcutoffs-history-graph'></div>";
		string c;
		for(auto &cutoff : cutoffs) {
			c+="\""+ToString(cutoff.first)+"\":"+ToString(cutoff.second)+",";
		}
		if(c.length()) c=c.substr(0,c.length()-1);
		string s="<script>window.wcs.cutoffs={"+c+"};window.wcs.cutoffshistorya=["+history+"]</script>";
		return LeftRight(l, r)+s;
	}
};
WCSPCutoffs wcspcutoffs;

class PlayersMod : public Module
{
public:
	double cutoff;
	uint icutoff;
	vector<WebPlayer> players;
	vector<WebPlayer> top_players;
	vector<WebTeam> teams;
	uint sim_id;
	uint comp_sim_id;
	uint samples;
	uint comp_samples;

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		sim_id = simheader.sim_id;
		comp_sim_id = simheader.comp_sim_id;
		samples = simheader.samples;
		comp_samples = simheader.comp_samples;
		cutoff = 0.0;
		icutoff = 0;

		players.resize(MAX_PLAYER_ID);
		teams.resize(MAX_TEAM_ID);

		auto * res = mysqlcon.select(("select players.p_id, teams.team_id, players_chances.season, times_top, players.name, teams.name, race, country, wcs_points, overall, vsP,vsT,vsZ,league, players_wcs_points.min, players_wcs_points.median, players_chances.sim_id, players_wcs_points.mean, players_wcs_points.mode, players_wcs_points.max, players_wcs_points.min_qualify, players_wcs_points.max_not_qualify, afk_times_top, afk_samps from players_chances right join players using(p_id) left join teams using(team_id) left join players_wcs_points on(players.p_id=players_wcs_points.p_id and players_wcs_points.sim_id=" + ToString(sim_id) + " and players_wcs_points.season=0) where length(players.name)>0 and ((players_wcs_points.season=0 and players_wcs_points.sim_id=" + ToString(sim_id) + ") or players_wcs_points.sim_id is null) and (players_chances.sim_id=" + ToString(sim_id) + " or players_chances.sim_id=" + ToString(comp_sim_id) + ")").c_str() DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint p_id = row[0];
			WebPlayer &p = players[p_id];
			uint tsim_id = row[16];
			if (tsim_id == comp_sim_id && tsim_id!=sim_id) {
				p.comp_times_top = row[3];
				p.afk_times_top_comp = row[22];
				p.afk_samps_comp = row[23];
				continue;
			}
			p.player_id = p_id;
			p.team_id = row[1];
			uint season = row[2];
			if (season == 0) {
				p.times_top = row[3];
			}
			p.name = row[4].cstr();
			p.race = (int)row[6];
			if (row[7].cstr() && strlen(row[7].cstr()) == 2)
				memcpy(p.country, row[7].cstr(), 2);
			else
				memcpy(p.country, "\0\0", 2);

			//p.wcs_points = row[8];
			p.ratings[(int)R] = row[9];
			p.ratings[(int)P] = row[10];
			p.ratings[(int)T] = row[11];
			p.ratings[(int)Z] = row[12];

			p.league = row[13];
			if (p.team_id) {
				teams[p.team_id].name = row[5].cstr();
				teams[p.team_id].team_id = p.team_id;
			}

			p.minpoints = row[14];
			p.medianpoints = row[15];
			p.meanpoints = row[17];
			p.modepoints = row[18];
			//players_wcs_points.max, players_wcs_points.min_qualify, players_wcs_points.max_not_qualify
			p.maxpoints = row[19];
			p.minpoints_qualify = row[20];
			p.maxpoints_notqualify = row[21];
			p.afk_times_top = row[22];
			p.afk_samps = row[23];
		}

		for (auto &p : players) {
			if (p.player_id) top_players.push_back(p);
		}
		for(auto &p : top_players) {
			teams[p.team_id].times_top += p.times_top;
			teams[p.team_id].comp_times_top += p.comp_times_top;
		}
		sort(top_players.begin(), top_players.end(), [](const WebPlayer &a, const WebPlayer &b) {
			if (a.times_top == b.times_top) {
				if (a.minpoints == b.minpoints) {
					return a.ratings[(int)R] > b.ratings[(int)R];
				}
				return a.minpoints > b.minpoints;
			}
			return a.times_top > b.times_top;
		});
		for(uint i=0;i<top_players.size();i++) {
			top_players[i].rank=i+1;
			players[top_players[i].player_id].rank=i+1;
		}

		cutoff = ((double)top_players[TOP_NUM_QUALIFY - 1].times_top / (double)samples * 100.0);
		icutoff = top_players[TOP_NUM_QUALIFY - 1].times_top;
	}

	virtual string InnerHtml()
	{
		string h;
		for (uint i = 0; i < 25 && i < top_players.size();i++) {
			auto &p = top_players[i];
			h += p.name.ToString() + "<br/>\n";
		}
		return h;
	}

	string phtml(uint p_id)
	{
		GetData(DEBUGARG);
		auto &p = players[p_id];
		return p.html(simheader.samples, teams[p.team_id]);
	}

	string pshorthtml(uint p_id)
	{
		GetData(DEBUGARG);
		auto &p = players[p_id];
		return p.shorthtml(simheader.samples, teams[p.team_id]);
	}

	string teamhtml(uint t_id)
	{
		GetData(DEBUGARG);
		return teams[t_id].html();
	}

	string pShortJSON(WebPlayer &p)
	{
		string ret;
		//if(p.aligulac_id==0) continue;
		string race = "R";
		if (p.race == P) race = "P";
		else if (p.race == T) race = "T";
		else if (p.race == Z) race = "Z";
		//string tname = "";
		//if (p.team_id) tname = teams[p.team_id].name.ToString();
		ret += "['" + p.name.ToString() + "'," + ToString(p.player_id) + "," + ToString(p.team_id) + ",'" + p.sCountry() + "','" + race + "'],";
		return ret;
	}

	string ShortJSON(uint max)
	{
		string ret = "[";
		max = min((uint)max, (uint)top_players.size());
		for (uint i = 0; i < max; i++) {
			auto &p = top_players[i];
			ret += pShortJSON(p);
		}
		if (ret.length())
			ret = ret.substr(0, ret.length() - 1);
		ret += "]";
		return ret;
	}

	string pMedJSON(WebPlayer &p, uint rank)
	{
		string ret;
		//if(p.aligulac_id==0) continue;
		string race = "R";
		if (p.race == P) race = "P";
		else if (p.race == T) race = "T";
		else if (p.race == Z) race = "Z";
		//string tname = "";
		//if (p.team_id) tname = teams[p.team_id].name.ToString();
		ret += "['" + p.name.ToString() + "'," + ToString(p.player_id) + "," + ToString(p.team_id) + ",'" + p.sCountry() + "','" + race + "'";
		ret += "," + ToString(p.minpoints) + "," + ToString(p.medianpoints) + "," + ToString(p.times_top) + "," + ToString(p.comp_times_top) + "," + PercentToJson(p.afk_times_top, p.afk_samps) + "," + PercentToJson(p.afk_times_top_comp, p.afk_samps_comp) + "," + ToString(rank) + "],";
		return ret;
	}

	string MedJSON(uint max)
	{
		string ret = "[";
		max = min((uint)max, (uint)top_players.size());
		for (uint i = 0; i < max; i++) {
			auto &p = top_players[i];
			ret += pMedJSON(p, i+1);
		}
		if (ret.length() > 0)
			ret = ret.substr(0, ret.length() - 1);
		ret += "]";
		return ret;
	}

	string TeamsJSON()
	{
		string ret = "{";
		for (uint i = 0; i<teams.size(); i++) {
			if (i != 0 && teams[i].team_id == 0) continue;
			auto &t = teams[i];
			ret += "'" + ToString(t.team_id) + "':{name:'" + t.name.ToString() + "',times_top:'" + ToString(t.times_top) + "'},";
		}
		if (ret.length() > 0)
			ret = ret.substr(0, ret.length() - 1);
		ret += "}";
		return ret;
	}

	virtual string MedHtml()
	{
		GetData(DEBUGARG);
		return "<script>window.wcs.sixteenthcutoff=" + PercentToJson(icutoff, samples) + ";window.wcs.teams=" + TeamsJSON() + ";\nwindow.wcs.playersmed=" + MedJSON(MAX_PLAYER_ID) + ";</script>";
	}
};
PlayersMod players;

class Tournaments : public Module
{
public:
	array<WebTournament, 256> tournies;
	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		wwglobals.GetData(DEBUGARG);
		tournies[1].name="";

		tournies[100].name="WCS Global Finals 2014";
		tournies[40].name="Hot6ix Cup";
		tournies[41].name = "IEM San Jose Stage 1";
		tournies[42].name = "IEM San Jose";
		tournies[43].name = "IEM Taipei Stage 1";
		tournies[44].name = "IEM Taipei";
		tournies[45].name = "IEM Katowice";

		tournies[46].name = "DreamHack Tours";
		//tournies[46].timestamp = Timestamp("May 8, 2015 10:00 {{Abbr /CDT");
		tournies[47].name = "DreamHack Valencia";
		//tournies[47].timestamp = Timestamp("July 16, 2015 11:30 {{Abbr /CEST");
		tournies[48].name = "DreamHack Stockholm";
		//tournies[48].timestamp = Timestamp("September 24, 2015 4:30 {{Abbr /CDT");

		tournies[49].name = "Copenhagen Spring";
		tournies[49].stream = "http://www.twitch.tv/copenhagengamessc2";

		for(uint i=0;i<25;i++) {
			tournies[i+101].name="Placeholder "+ToString(i+1);
		}
		for(uint s=0;s<3;s++) {
			tournies[s*12+1].name="GSL Qualifiers Season "+ToString(s+1);
			tournies[s*12+2].name="GSL Code A Season "+ToString(s+1);
			tournies[s*12+3].name="GSL Code S Season "+ToString(s+1);
			tournies[s*12+4].name="WCS Qualifiers Season "+ToString(s+1);
			tournies[s*12+5].name="WCS Challenger Season "+ToString(s+1);
			tournies[s*12+6].name="WCS Premier Season "+ToString(s+1);
			if (simheader.sim_id <= 1353 && simheader.sim_id!=0) {
				tournies[s * 12 + 7].name = "StarLeague Season " + ToString(s + 1);
				tournies[s * 12 + 8].name = "StarLeague Qualifiers Season " + ToString(s + 1);
			}
			else {
				tournies[s * 12 + 7].name = "StarLeague Qualifiers Season " + ToString(s + 1);
				tournies[s * 12 + 8].name = "StarLeague Season " + ToString(s + 1);
			}
		}
		
		tournies[14].stream = tournies[3].stream = "http://www.twitch.tv/gsl";
		tournies[6].stream = "http://www.twitch.tv/wcs";

		tournies[4].timestamp = Timestamp("January 20, 2015 7:00 {{Abbr /CDT");//Timestamp("January 7, 2015 12:00 {{Abbr /CDT");//1420588800+3600*23;
		tournies[4].stream="http://www.twitch.tv/basetradetv";
		tournies[19].stream =  tournies[7].stream = "http://www.twitch.tv/sc2starleague";
		tournies[20].stream = tournies[8].stream = "http://www.twitch.tv/sc2starleague";
		tournies[43].stream = "http://www.twitch.tv/esltv_sc2";
		tournies[44].stream = "http://www.twitch.tv/esltv_sc2";
		tournies[45].stream = "http://www.twitch.tv/esltv_sc2";

		//WCS S2
		//tournies[16].timestamp = Timestamp("April 17, 2015 18:00 {{Abbr /CEST");//wcs eu quals day 4
		//tournies[16].timestamp = Timestamp("April 18, 2015 17:00 {{Abbr /CDT");//am q 4
		//tournies[16].timestamp = Timestamp("April 20, 2015 19:00 {{Abbr /CDT");//la day 1
		//tournies[16].timestamp = Timestamp("April 28, 2015 18:00 {{Abbr /CDT");//la quals
		//tournies[16].timestamp = Timestamp("April 11, 2015 21:00 {{Abbr /CDT");// Timestamp("April 11, 2015 18:00 {{Abbr /CEST");//"March 14, 2015 05:00 {{Abbr /CDT"//"April 11, 2015 18:00 {{Abbr /CEST"
		//tournies[17].timestamp = Timestamp("April 26, 2015 05:00 {{Abbr /CDT");//TW challenger
		//tournies[18].timestamp = Timestamp("May 16, 2015 17:00 {{Abbr /CDT");
		//tournies[20].timestamp = Timestamp("April 16, 2015 04:30 {{Abbr /CDT");
		//tournies[25].timestamp = Timestamp("June 11, 2015 23:00 {{Abbr /CDT");//code b season 3
		
		tournies[50].name = "Gfinity Spring 1";

		tournies[51].name = "KeSPA Cup 1";
		tournies[51].stream = "http://www.twitch.tv/sc2proleague";
		//tournies[51].timestamp = Timestamp("May 3, 2015 05:00 {{Abbr /CDT");

		tournies[52].name = "Gfinity Spring 2";
		//tournies[52].timestamp = Timestamp("May 1, 2015 13:00 {{Abbr /CDT");
		tournies[53].name = "Gfinity Summer 1";
		//tournies[53].timestamp = Timestamp("August 7, 2015 10:00 {{Abbr /CDT");
		tournies[54].name = "Gfinity Summer 2";
		//tournies[54].timestamp = Timestamp("September 5, 2015 10:00 {{Abbr /CDT");

		tournies[55].name = "3rd Hong Kong Esports Tournament";
		tournies[56].name = "IEM Shenzhen";
		tournies[57].name = "IEM Shenzhen Open Bracket";
		//tournies[56].timestamp = Timestamp("July 17, 2015 9:15 {{Abbr /CST");
		//tournies[57].timestamp = Timestamp("July 16, 2015 9:30 {{Abbr /CST");
		tournies[58].name = "KeSPA Cup 2";
		//tournies[58].timestamp = Timestamp("July 6, 2015 04:00 {{Abbr /CDT");
		//tournies[31].timestamp = Timestamp("June 25, 2015 01:00 {{Abbr /CDT");//SSL S3 quals

		tournies[198].name = "End of WCS Wars Season "+ToString(wwglobals.season_num);
		tournies[198].timestamp = wwglobals.season_end;
		if(wwglobals.season_num==2) {
			tournies[199].name = "Start of WCS Wars Season 3";
			tournies[199].timestamp = wwglobals.season_end+86400;
		}
		/*if ((uint)time(0) < Timestamp("April 6, 2015 22:00 {{Abbr /CDT")) {
			tournies[198].name = "End of WCS Wars Season 1";
			tournies[198].timestamp = Timestamp("April 5, 2015 22:00 {{Abbr /CDT");
			tournies[199].name = "Start of WCS Wars Season 2";
			tournies[199].timestamp = Timestamp("April 6, 2015 22:00 {{Abbr /CDT");
		}*/
		//tournies[197].name = "KeSPA Cup Qualifiers";
		//tournies[197].timestamp = Timestamp("June 24, 2015 02:00 {{Abbr /CDT");
		//tournies[190].name = "Gfinity Qualifiers";
		//tournies[190].timestamp = Timestamp("April 14, 2015 12:00 {{Abbr /CDT");
		
		//tournies[18].timestamp = Timestamp("June 28, 2015 12:00 {Abbr /CDT");

		tournies[59].name = "ASUS ROG Summer";
		//tournies[59].timestamp = Timestamp("July 30, 2015 10:00 {{Abbr /CDT");
		tournies[59].stream = "http://www.twitch.tv/rogtournament_sc2";

		tournies[60].name = "IEM Gamescom";
		//tournies[60].timestamp = Timestamp("August 6, 2015 10:00 {{Abbr /CDT");
		tournies[62].name = "IEM Gamescom Open Bracket";
		//tournies[62].timestamp = Timestamp("August 5, 2015 10:00 {{Abbr /CDT");

		tournies[61].name = "MSI Masters";
		//tournies[61].timestamp = Timestamp("August 28, 2015 10:00 {{Abbr /CDT");

		tournies[63].name = "KeSPA Cup 3";
		tournies[64].name = "4Gamers";

		/*tournies[63].name = "IEM Gamescon Open Bracket";
		tournies[63].timestamp = Timestamp("August 5, 2015 10:00 {{Abbr /CDT");
		tournies[64].name = "IEM Gamescon";
		tournies[64].timestamp = Timestamp("August 6, 2015 10:00 {{Abbr /CDT");*/
		//WCS S3
		//tournies[28].timestamp = Timestamp("July 18, 2015 12:00 {{Abbr /CEST");//WCS TW qualifier
		//tournies[29].timestamp = Timestamp("July 12, 2015 04:00 {{Abbr /CDT");//WCS SEA chal
		tournies[29].stream = "http://www.twitch.tv/wcs";
		//tournies[30].timestamp = Timestamp("August 13, 2015 15:00 {{Abbr /CDT");//WCS prem ro32
		//tournies[32].timestamp = Timestamp("July 2, 2015 04:30 {{Abbr /CDT");//SSL S3
		tournies[26].stream = "http://www.twitch.tv/gsl";
		tournies[27].stream = "http://www.twitch.tv/gsl";
		tournies[32].stream = "http://www.azubu.tv/EN_SC2_Starleague";
		tournies[30].stream = "http://www.twitch.tv/wcs";
		
		for(uint i=0;i<tournies.size();i++) {
			if(tournies[i].name.length()>0) tournies[i].id=i;
		}
	}

	virtual string InnerHtml()
	{
		return string();
	}

	string thtml(uint tid)
	{
		GetData(DEBUGARG);
		if(tid>=tournies.size()) return ToString(tid);
		return tournies[tid].html();
	}
};
Tournaments tournaments;

const int NUM_HEADBANDS = 4;
class Headbands : public Module
{
public:
	vector<Headband> headbands;
	/*uint num1;
	uint num2;
	uint num1_defenses;
	uint num2_defenses;*/
	uint hbs[NUM_HEADBANDS];
	uint defenses[NUM_HEADBANDS];

	Headbands()
	{
		/*num1 = 0;
		num2 = 0;
		num1_defenses = 0;
		num2_defenses = 0;*/
		for (int i = 0; i < NUM_HEADBANDS; i++) {
			hbs[i] = 0;
			defenses[i] = 0;
		}
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		auto &p = players.players;
		auto *res = mysqlcon.select("select defender_id,challenger_id,headband,defended,unix_timestamp(played_time),t_id,current from headbands order by played_time asc" DEBUGARGS);

		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint defender_id = row[0];
			uint challenger_id = row[1];
			int headband = row[2];
			int defended = row[3];
			uint timestamp = row[4];
			uint t_id = row[5];
			//int current = row[6];

			Headband hb(challenger_id, defender_id, headband, t_id, timestamp, defended);
			headbands.push_back(hb);

			uint winner = defender_id;
			uint loser = challenger_id;
			if (!defended) {
				winner = challenger_id;
				loser = defender_id;
			}
			int other = 0;
			for (int i = 0; i < NUM_HEADBANDS; i++) {
				if (hbs[i] == challenger_id) {
					other = i + 1;
				}
			}
			hbs[headband - 1] = winner;
			if (other) hbs[other - 1] = loser;
		}

		for (int i = 0; i < NUM_HEADBANDS; i++) {
			if(hbs[i]) p[hbs[i]].headband = i + 1;
		}
		for (auto h : headbands) {
			if (h.defended) {
				defenses[h.headband - 1]++;
			} else {
				defenses[h.headband - 1] = 0;
			}
		}
	}

	virtual string InnerHtml()
	{
		string l;
		string r;
		ShowMore showmore;
		for(int i=0;i<NUM_HEADBANDS;i++) {
			l += "<mdiv class='headband headband-"+ToString(i+1)+"'>" + players.phtml(hbs[i]) + " is the current #"+ToString(i+1)+" Headband holder with " + ToString(defenses[i]) + " defenses!</mdiv><br/>\n";
		}
		for(uint i=(uint)headbands.size()-1;i<headbands.size();i--) {
			auto &h=headbands[i];
			if(i==headbands.size()-4) r+=showmore.Start("Defense History");
			if(h.defended) {
				r += "<mdiv class='headband-history headband-defense'>"+players.phtml(h.defender)+" defended the #"+ToString(h.headband)+" from "+players.phtml(h.challenger)+" at "+tournaments.thtml(h.t_id)+"</mdiv>";
			} else {
				r += "<mdiv class='headband-history headband-win'>"+players.phtml(h.challenger)+" won the #"+ToString(h.headband)+" from "+players.phtml(h.defender)+" at "+tournaments.thtml(h.t_id)+"</mdiv>";
			}
		}
		r += showmore.End();
		return LeftRight(l,r);
	}

	virtual string ToJSON(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		//string ret = "<script>window.wcs.headbands=[{id:" + ToString(num1) + ",defenses:" + ToString(num1_defenses) + "},{id:" + ToString(num2) + ",defenses:" + ToString(num2_defenses) + "}]</script>";
		string ret = "<script>window.wcs.headbands=[";
		for (int i = 0; i < NUM_HEADBANDS; i++) {
			ret += "{id:" + ToString(hbs[i]) + ",defenses:" + ToString(defenses[i]) + "},";
		}
		ret = ret.substr(0, ret.length() - 1);
		ret += "]</script>";
		return ret;
	}
};
Headbands headbands;

class Events : public Module
{
public:
	vector<WebEventGroup> evgs;

	void MakeOr(vector<WebEventGroup> &newevgs, const WebEventGroup &evg)
	{
		if (evg.size()+1 >= MAX_EVENTS_GROUP) return;
		if (evg.size() >= 5) return;

		int minplace = 255;
		int maxplace = 0;
		for (uint e = 0; e<evg.size(); e++) {
			minplace = min((int)evg.events[e].placing, minplace);
			maxplace = max((int)evg.events[e].placing, maxplace);
		}
		//uint samps = simheader.samples;

		for (auto &eg : evgs) {
			if (eg.size() != 1) continue;
			if (eg.type != NORMAL_EVG_TYPE) continue;
			if (eg.pid != evg.pid) continue;
			auto &e1 = eg.events[0];
			auto &e2 = evg.events[0];
			if (e1.placing >= SPECIAL_EVENT) continue;
			if (e1.t_id != e2.t_id) continue;//I can't do this since they aren't mutually exclusive events :(
			if (e1.t_id != e2.t_id && (e1.placing!=e2.placing || evg.size()>1) ) continue;
			if (e1.t_id < e2.t_id) continue;
			if (e1.p_id != e2.p_id) continue;
			if (e1.t_id == e2.t_id && e1.placing >= minplace && e1.placing <= maxplace) continue;//itself?
			if (e1.t_id==e2.t_id && (int)e1.placing <= maxplace) continue;
			if ((int)e1.placing * 2 < minplace || maxplace * 2 < (int)e1.placing) continue;

			WebEventGroup tevg = evg;
			tevg.AddEvent(e1);
			tevg.type = OR_EVG_TYPE;
			tevg.hits += eg.hits;
			tevg.misses += eg.misses;
			std::sort(tevg.events.begin(), tevg.events.end(), [](const EventCounter &a, const EventCounter &b)
			{
				if (a.p_id != b.p_id) return a.p_id > b.p_id;
				if (a.placing != b.placing) return a.placing < b.placing;
				return a.t_id > b.t_id;
			});

			if (tevg.hits + tevg.misses == 0) continue;
			newevgs.push_back(tevg);
			MakeOr(newevgs, tevg);
		}
	}

	void MakeOrs()
	{
		vector<WebEventGroup> newevgs;
		for (auto &eg : evgs) {
			if (eg.size() != 1) continue;
			if (eg.type != NORMAL_EVG_TYPE) continue;
			//if (eg.pid != eg.events[0].p_id) continue;
			MakeOr(newevgs, eg);
		}
		for (auto &eg : newevgs) evgs.push_back(eg);
	}

	void MakeNegative(vector<WebEventGroup> &newevgs, WebEventGroup evg)
	{
		uint samps = simheader.samples;
		uint times_top = players.players[evg.pid].times_top;
		/*for (uint i = 1; i < evg.size(); i++) {
			if (evg.events[0].t_id != evg.events[i].t_id) return;
		}*/
		if (evg.type == NORMAL_EVG_TYPE) {
			evg.type = NEG_EVG_TYPE;
		}
		else if (evg.type == OR_EVG_TYPE) {
			//return;
			evg.type = NEG_AND_EVG_TYPE;
		}
		else return;

		evg.hits = times_top - evg.hits;
		evg.misses = (samps - times_top) - evg.misses;

		if (evg.hits + evg.misses == 0) return;
		newevgs.push_back(evg);
	}

	void MakeNegatives()
	{
		vector<WebEventGroup> newevgs;
		for (auto &eg : evgs) {
			//if (eg.size() != 1) continue;
			//if (eg.type != 1) continue;
			if (eg.size() == 1 && eg.events[0].placing >= SPECIAL_EVENT) continue;
			if (eg.type == NEG_EVG_TYPE || eg.type==NEG_AND_EVG_TYPE) continue;
			MakeNegative(newevgs, eg);
		}
		for (auto &eg : newevgs) evgs.push_back(eg);
	}

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		uint min_samps = max(50u, simheader.samples / 10000u);//must be at least 0.01% prob
		auto *res = mysqlcon.select(("select evg_id, hits, misses, events.p_id, placing, size, event_groups.p_id, t_id from event_groups join events using(evg_id) where sim_id="+ToString(simheader.sim_id)+" and evg_id between "+ToString(simheader.sim_id*EVG_ID_MULT)+" and "+ToString((simheader.sim_id+1)*EVG_ID_MULT)+" and total>"+ToString(min_samps)+" order by evg_id").c_str() DEBUGARGS);

		WebEventGroup evg;
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			evg.evg_id = row[0];
			evg.hits = row[1];
			evg.misses = row[2];
			evg.pid = row[6];
			uint pid = evg.pid;

			EventCounter ev;
			ev.p_id = (uint)row[3];
			ev.t_id = (uint)row[7];
			ev.placing = (uint)row[4];
			//ev.h = Event::Hash(ev);
			evg.AddEvent(ev);

			uint s = row[5];
			if (evg.size() == s) {
				std::sort(evg.events.begin(), evg.events.end(), [pid](const EventCounter &a, const EventCounter &b) {
					if (a.p_id == 0 || b.p_id == 0) {
						return a.p_id > b.p_id;
					}
					if (a.p_id != b.p_id) {
						if (a.p_id == pid) return true;
						if (b.p_id == pid) return false;
					}
					if (a.t_id != b.t_id) return a.t_id < b.t_id;
					return a.placing < b.placing;
				});
				evgs.push_back(evg);
				evg = WebEventGroup();
			}
		}

		MakeOrs();
		MakeNegatives();
	}

	virtual string InnerHtml()
	{
		return string();
	}
};
Events events;

//globals for SimHeader, Players, Tournaments, Events, UpcomingMatches, WWGlobals, FinalFacingStats, Headbands, ForeignHope, PointCutoffs...
//	-maybe these shouldn't output html, but use different modules to read the data from them and output html? for smaller modules it's ok to combine
//maybe globals - BiggestWinnersLosers, TournamentWinners, TournamentPlacings, GroupOfDeath
//TournamentGains, PlayerHistory, PlayerHistoryMedian, PlayerEvents, TournamentEvents, PlayerSetEvents, PlayerInfo, PlayerUpcomingMatches, TournamentUpcomingMatches, PlayerSetUpcomingMatches
//	-instead of having PlayerEvents/TournamentEvents and PlayerUpcomingMatches/TournamentUpcomingMatches/PlayerSetUpcomingMatches I can just have a generic event filter object that I apply to the EventsHtml object or UpcomingMatchesHtml object...
//		-use a few set variables to filter with or a lambda?

class TopPlayers : public Module
{
public:
	uint num;
	TopPlayers(uint top_num)
	{
		num = top_num;
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		string ret;
		//ret += players.MedHtml();

		string jsonplayers;
		uint samps=simheader.samples;
		//uint comp_samps=simheader.comp_samples;

		for (uint i = 0; i < num && i < players.top_players.size(); i++) {
			auto &p=players.top_players[i];
			string race = "R";
			if (p.race == P) race = "P";
			else if (p.race == T) race = "T";
			else if (p.race == Z) race = "Z";
			string team_name;
			if (p.team_id) team_name = players.teams[p.team_id].name.ToString();
			//ret += "['" + p.name.ToString() + "'," + ToString(p.player_id) + "," + ToString(p.team_id) + ",'" + p.sCountry() + "','" + race + "'";
			//ret += "," + ToString(p.minpoints) + "," + ToString(p.medianpoints) + "," + ToString(p.times_top) + "," + ToString(p.comp_times_top) + ","+PercentToJson(afk_times_top,akf_samps)+","+PercentToJson(afk_times_top_comp,afk_samps_comp)+"," + ToString(i + 1) + "],";
			jsonplayers += "[\"" + jsonescape(p.name.ToString(),1024) + "\"," + ToString(p.player_id) + "," + ToString(p.team_id) + ",\"" + p.sCountry() + "\",\"" + race + "\"";
			jsonplayers += "," + ToString(p.minpoints) + "," + ToString(p.medianpoints) + "," + ToString(p.times_top) + "," + ToString(p.comp_times_top) + ","+PercentToJson(p.afk_times_top,p.afk_samps)+","+PercentToJson(p.afk_times_top_comp,p.afk_samps_comp)+"," + ToString(p.rank) + ",\""+jsonescape(team_name, 1024)+"\"],";
		}
		if(jsonplayers.length()) jsonplayers = jsonplayers.substr(0,jsonplayers.length()-1);
		ret += "<script>window.wcs.sixteenthcutoff=" + PercentToJson(players.icutoff, samps) + ";\nwindow.wcs.topplayers=[" + jsonplayers + "];</script>";
		ret += Table("top-players-table");
		if(globalcontext.r!=Renderer::bbcode) return ret;
		ret+="<mdiv style='padding:6px;'>";
		for (uint i = 0; i < num*2 && i < players.top_players.size(); i++) {
			//if(i==num) ret += "<br/>";
			auto &p=players.top_players[i];
			string race = "r";
			if (p.race == P) race = "p";
			else if (p.race == T) race = "t";
			else if (p.race == Z) race = "z";
			string team_name;
			if (p.team_id) team_name = " ("+players.teamhtml(p.team_id)+")";
			//ret += "<mdiv>[*] :"+p.sCountry()+": :"+race+": "+players.pshorthtml(p.player_id)+team_name+", is at "+RenderPercent(p.times_top, samps, players.cutoff)
			//+", Min WCS Points: [b]"+ToString(p.minpoints)+"[/b], Median WCS Points: [b]"+ToString(p.medianpoints)+"[/b]</mdiv>";
			ret += "<mdiv>[*] "+players.pshorthtml(p.player_id)+" is at "+RenderPercent(p.times_top, samps, players.cutoff)
				+ ", " + RenderPercent(p.afk_times_top, p.afk_samps, players.cutoff) + " AFK Chances, Min WCS Points: [b]" + ToString(p.minpoints) + "[/b], Median Points: [b]" + ToString(p.medianpoints) + "[/b]</mdiv>";
		}
		ret+="</mdiv>";
		return ret;
	}
};

class PlayerSetTable : public Module
{
public:
	vector<uint> pids;
	PlayerSetTable(vector<uint> &PIDs) : pids(PIDs)
	{
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		string ret;
		//ret += players.MedHtml();

		string jsonplayers;
		uint samps=simheader.samples;
		//uint comp_samps=simheader.comp_samples;

		for(auto i : pids) {
			auto &p=players.players[i];
			string race = "R";
			if (p.race == P) race = "P";
			else if (p.race == T) race = "T";
			else if (p.race == Z) race = "Z";
			string team_name;
			if (p.team_id) team_name = players.teams[p.team_id].name.ToString();
			jsonplayers += "[\"" + jsonescape(p.name.ToString(),1024) + "\"," + ToString(p.player_id) + "," + ToString(p.team_id) + ",\"" + p.sCountry() + "\",\"" + race + "\"";
			jsonplayers += "," + ToString(p.minpoints) + "," + ToString(p.medianpoints) + "," + ToString(p.times_top) + "," + ToString(p.comp_times_top) + ","+PercentToJson(p.afk_times_top,p.afk_samps)+","+PercentToJson(p.afk_times_top_comp,p.afk_samps_comp)+"," + ToString(p.rank) + ",\""+jsonescape(team_name, 1024)+"\"],";
		}
		if(jsonplayers.length()) jsonplayers = jsonplayers.substr(0,jsonplayers.length()-1);
		ret += "<script>window.wcs.sixteenthcutoff=" + PercentToJson(players.icutoff, samps) + ";\nwindow.wcs.topplayers=[" + jsonplayers + "];</script>";
		ret += Table("top-players-table");
		return ret;
	}
};

class PlayerHistoryGraph : public Module
{
public:
	vector<uint> pids;//make these variables private so I can have it smart enough to avoid duplicate items, and figure out how to do multiple datetime bounds
	string spids;//is it worth compressing the history json into a single array instead of an array of arrays? maybe
	string histories;
	uint start;
	uint end;
	bool outputted;
	int mode;

	PlayerHistoryGraph()
	{
		outputted=false;
		mode=0;
	}
	
	~PlayerHistoryGraph()
	{
		if(pids.size()>0 && outputted==false) {
			throw rayexception("unoutputted PlayerHistoryGraph data" DEBUGARGS);
		}
	}

	static string GetQueryString(uint start, uint end, string spids)
	{
		string ret;
		ret = "select players_chances.p_id, times_top, players_chances.sim_id, sims.size, unix_timestamp(sims.created), min, median, mean, afk_times_top, afk_samps from players_chances join sims using(sim_id) join players_wcs_points on (players_wcs_points.sim_id=sims.sim_id and players_wcs_points.p_id=players_chances.p_id) where players_chances.p_id in(" + spids + ") and sims.created>'2014-11-01' and sims.created between from_unixtime(" + ToString(start) + ")-interval 1 second and from_unixtime(" + ToString(end) + ")+interval 1 second and (sims.size>=" + ToString(HISTORY_MIN_SIZE) + " or players_chances.sim_id=" + ToString(simheader.sim_id) + " or players_chances.sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac=" + ToString(USE_ALIGULAC) + " and (sims.type=" + ToString(SIM_TYPE) + " or sims.sim_id=" + ToString(simheader.sim_id) + " or sims.sim_id=" + ToString(simheader.comp_sim_id) + ") order by players_chances.p_id,created";
		return ret;
	}

	virtual void RunQuery()
	{
		outputted=false;
		histories="";
		spids="";
		if(pids.size()==0) return;
		for(auto p:pids) spids+=ToString(p)+",";
		if(spids.length()==0) return;
		spids=spids.substr(0,spids.length()-1);
		
		auto * res = mysqlcon.select(GetQueryString(start, end, spids).c_str() DEBUGARGS);
		
		string history=",";
		uint last_pid=0;
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint times_top=row[1];
			uint size=row[3];
			uint created=row[4];

			uint minpoints=row[5];
			uint medianpoints=row[6];
			uint meanpoints=row[7];
			uint afk_times_top=row[8];
			uint afk_samps=row[9];
			if(afk_samps==0) {
				afk_times_top=0;
				afk_samps=100;
			}

			double prob = (double)times_top/(double)size *100.0;
			double afk_prob = (double)afk_times_top/(double)afk_samps *100.0;
			if(pid!=last_pid) {
				history=history.substr(0,history.length()-1)+"],"+ToString(pid)+":['"+jsonescape(players.players[pid].name.ToString(),1024)+"',";
				last_pid=pid;
			}
			if(mode==0) {
				history+="["+ToString(prob)+","+ToString(created)+"],";
			} else if(mode==1) {
				history+="["+ToString(prob)+","+ToString(created)+","+ToString(afk_prob)+"],";
			} else if(mode==2) {
				//uint value=PointsValue(minpoints,medianpoints);
				history+="["+ToString(prob)+","+ToString(created)+","+ToString(afk_prob)+","+ToString(minpoints)+","+ToString(medianpoints)+","+ToString(meanpoints)+"],";
			}
		}
		if(history.length()<=1) return;
		history="{"+history.substr(2);
		history=history.substr(0,history.length()-1)+"]}";
		histories=history;
	}
	
	virtual string InnerHtml()
	{
		outputted=true;
		return "\n<script>window.wcs.player_historiesa.push("+histories+");</script>\n";//"<div class='player-history-graph' data-pids='"+spids+"'></div>";
	}
};
PlayerHistoryGraph playerhistorygraphsdata;

class ChancesChange
{
public:
	uint pid;
	double change;
	
	ChancesChange(uint PID, double Change) {
		pid=PID;
		change=Change;
	}

	ChancesChange()
	{
		pid=0;
		change=0;
	}
};
class BiggestWinnersLosers : public Module
{//this is the base class for both, since I have to query for the graphs it's more efficient to do them together
public:
	vector<ChancesChange> winners;
	vector<ChancesChange> losers;
	//PlayerHistoryGraph graphs;

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		auto samps=simheader.samples;
		auto compsamps=simheader.comp_samples;
		for(auto &p: players.top_players) {
			double change = ((double)p.times_top/(double)samps)-((double)p.comp_times_top/(double)compsamps);
			change*=100.0;
			if(abs(change)<0.01) continue;
			ChancesChange c(p.player_id, change);
			if(change<0.0) losers.push_back(c);
			else if(change>0.0) winners.push_back(c);
		}

		std::sort(winners.begin(), winners.end(), [](const ChancesChange a, const ChancesChange b) {
			return a.change>b.change;
		});
		std::sort(losers.begin(), losers.end(), [](const ChancesChange a, const ChancesChange b) {
			return a.change<b.change;
		});
		if(winners.size()>50) winners.resize(50);
		if(losers.size()>50) losers.resize(50);
		if(winners.size()+losers.size() == 0) return;
		
		for(uint i=0;i<7 && i<winners.size();i++) playerhistorygraphsdata.pids.push_back(winners[i].pid);
		for(uint i=0;i<7 && i<losers.size();i++) playerhistorygraphsdata.pids.push_back(losers[i].pid);
		playerhistorygraphsdata.start=simheader.comp_created;
		playerhistorygraphsdata.end=simheader.created;
		//playerhistorygraphsdata.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		//return "\n<script>window.wcs.player_historiesa.push("+graphs.histories+");</script>\n";
		return string();
	}

	string ChangesHtml(vector<ChancesChange> &changes)
	{
		string ret;
		ShowMore showmore;
		auto samps=simheader.samples;
		auto compsamps=simheader.comp_samples;
		auto cutoff=players.cutoff;
		for(uint i=0;i<changes.size() && i<50;i++) {
			auto &c=changes[i];
			auto &p=players.players[c.pid];
			if(i==7) ret+=showmore.Start((c.change>0.0?"Winners":"Losers"));
			ret += "<mdiv class='big-changer'>"+players.phtml(c.pid)+" went "+(c.change>0.0?"up":"down")+" by "+RenderPercentD(std::abs(c.change), std::abs(c.change-1.0), (c.change<0.0?-1:1) )+", going from "+RenderPercent(p.comp_times_top, compsamps, cutoff)+" to "+RenderPercent(p.times_top, samps, cutoff)+"</mdiv>";
		}
		ret+=showmore.End();
		return ret;
	}

	string WinnersHtml()
	{
		GetData(DEBUGARG);
		string pids;
		for(uint i=0;i<7 && i<winners.size();i++) pids+=ToString(winners[i].pid)+",";
		return LeftRight(ChangesHtml(winners), "<div class='graph player-history-graph' data-pids='"+pids+"'></div>");
	}

	string LosersHtml()
	{
		GetData(DEBUGARG);
		string pids;
		for(uint i=0;i<7 && i<losers.size();i++) pids+=ToString(losers[i].pid)+",";
		return LeftRight(ChangesHtml(losers), "<div class='graph player-history-graph' data-pids='"+pids+"'></div>");
	}
};
BiggestWinnersLosers biggest_winners_losers;

class UpcomingMatchMod
{
public:
	uint timestamp;
	vector<uint> pids;
	uint t_id;
	uint upm_id;
	uint round;
	bool estimated_time;
	vector<WebEventGroup> evgs;
	bool important;
	double score;

	UpcomingMatchMod()
	{
		estimated_time=false;
		important=false;
		score = 0.0;
	}

	string RoundOf()
	{
		string sround="round of "+ToString(round);
		if(round==8) sround="quarterfinals";
		else if(round==4) sround="semifinals";
		else if(round==2) sround="finals";
		return sround;
	}

	string Header()
	{
		string ret;
		for(uint p=0;p<pids.size();p++) {
			if(p+1==pids.size() && pids.size()==2) ret += "vs ";
			else if(p+1==pids.size()) ret += "and ";
			ret += players.phtml(pids[p]);
			if(p+1<pids.size() && pids.size()>2) ret +=", ";
			else if(p+1<pids.size()) ret+=" ";
		}
		ret+=" ";
		//if(globalcontext.r==Renderer::bbcode) ret += CountdownBB(timestamp);
		/*else */
		if(timestamp==0) ret += "(unscheduled)";
		else ret += CountdownHtml(timestamp);
		return ret;
	}

	bool MustWin(uint pid)
	{
		//uint samps = simheader.samples;
		//double chances = (double)players.players[pid].times_top / (double)samps *100.0;
		//if (chances < 0.1) return false;
		WebEventGroup win, lose;
		GetEvents(pid, win, lose);
		if (win.hits + win.misses == 0 || lose.hits + lose.misses == 0) return false;//if it can't find either event then give up
		double when_win = (double)win.hits / (double)(win.hits+win.misses) *100.0;
		double when_lose = (double)lose.hits / (double)(lose.hits+lose.misses) *100.0;
		if (when_win < 1.0) return false;
		if (when_win > (when_lose * 5)) return true;
		return false;
	}

	string MustWinsText()
	{
		string ret;
		vector<uint> must_win_pids;
		for (auto p : pids) {
			if (MustWin(p)) must_win_pids.push_back(p);
		}
		if (must_win_pids.size() == 0) return "";

		for (uint p = 0; p<must_win_pids.size(); p++) {
			if (must_win_pids.size()>1 && p + 1 == must_win_pids.size()) ret += "and ";
			ret += players.pshorthtml(must_win_pids[p]);
			if (p + 1<must_win_pids.size() && must_win_pids.size()>2) ret += ", ";
			else if (p + 1<must_win_pids.size()) ret += " ";
		}
		ret += " must win this match!";
		return "<div class='must-wins'>"+ret+"</div>";
	}

	void GetEvents(uint pid, WebEventGroup &win, WebEventGroup &lose)
	{
		for (auto &evg : evgs) {
			if (evg.events[0].p_id != pid) continue;
			if (evg.type != NORMAL_EVG_TYPE) continue;
			if (evg.pid != pid) continue;
			if (evg.size() != 1) continue;
			if (evg.events[0].placing == UPCOMING_WIN) win = evg;
			else if (evg.events[0].placing == UPCOMING_LOSE) lose = evg;
		}
	}

	string PlayerSection(uint pid)
	{
		WebEventGroup win;
		WebEventGroup lose;
		//string whenwin;
		//string whenlose;
		uint samps=simheader.samples;
		double chances=(double)players.players[pid].times_top/(double)samps *100.0;

		/*for (auto &evg : evgs) {
			if (evg.events[0].p_id != pid) continue;
			if (evg.type != NORMAL_EVG_TYPE) continue;
			if (evg.pid != pid) continue;
			if (evg.size() != 1) continue;
			string s;
			string result;
			if (evg.events[0].placing == UPCOMING_LOSE) {
				lose = evg;
				result = "lose";
				//s += "<div><span style='white-space:nowrap;'>" + RenderPercent(evg.hits + evg.misses, samps, 100.0) + " of the time they " + result + " this match</span>";
			}
			if (evg.events[0].placing == UPCOMING_WIN) {
				win = evg;
				result = "win";
				//s += "<div><span style='white-space:nowrap;'>" + RenderPercent(evg.hits + evg.misses, samps, 0.0) + " of the time they " + result + " this match</span>";
			}
			//s+="<div><span style='white-space:nowrap;'>"+RenderPercent(evg.hits+evg.misses, samps, 50.0)+" of the time "+"they"+" "+result+" this match</span>";
			string updown;
			double afterchances = (double)evg.hits / ((double)evg.hits + evg.misses);
			if (afterchances > chances) updown = " up";
			if (afterchances < chances) updown = " down";
			updown = "";
			//s += " <span style='white-space:nowrap;'>and their Blizzcon Chances go" + updown + " to " + RenderPercent(evg.hits, evg.hits + evg.misses, chances) + ".</span></div>";
			if (evg.events[0].placing == UPCOMING_WIN) whenwin = s;
			if (evg.events[0].placing == UPCOMING_LOSE) whenlose = s;
		}*/
		GetEvents(pid, win, lose);
		string ret;
		//ret+="<div>"+players.pshorthtml(pid)+" is at "+RenderPercent(players.players[pid].times_top, samps, players.cutoff)+" Blizzcon Chances.</div>"+whenwin+whenlose+"<rhr></rhr>";
		ret += "<div>" + players.pshorthtml(pid) + " <span style='white-space:nowrap;'>is at " + RenderPercent(players.players[pid].times_top, samps, players.cutoff) + " Blizzcon Chances</span>";
		if (win.hits + win.misses) ret+=", <span style='white-space:nowrap;'>with a " + RenderPercent(win.hits + win.misses, samps, 50.0) + " chance to win this match</span>";
		else if (lose.hits + lose.misses) ret += ", <span style='white-space:nowrap;'>with a " + RenderPercent(lose.hits + lose.misses, samps, 50.0) + " chance to lose this match</span>";
		ret += ".</div>";
		//double upto = (double)win.hits / (double)(win.hits+win.misses);
		//double downto = (double)lose.hits / (double)(lose.hits + lose.misses);
		if (win.hits + win.misses) {
			ret += "<div><span style='white-space:nowrap;'>If they win</span> <span style='white-space:nowrap;'>their Blizzcon Chances go up to " + RenderPercent(win.hits, win.hits + win.misses, chances) + "</span>";
			if (lose.hits + lose.misses) ret += ", <span style='white-space:nowrap;'>or " + RenderPercent(lose.hits, lose.hits + lose.misses, chances) + " if they lose</span>";
			ret += ".</div>";
		}
		else if (lose.hits + lose.misses) ret += "<div><span style='white-space:nowrap;'>If they lose</span> <span style='white-space:nowrap;'>their Blizzcon Chances go down to " + RenderPercent(lose.hits, lose.hits + lose.misses, chances) + ".</span></div>";
		else ret += "<div>---------------</div>";
		return ret;
	}

	string Html(DEBUGARGDEF)
	{
		string ret;
		string retl;
		string retr;
		string h=Header();
		string importants;
		string headbands;
		importants = MustWinsText();
		if(important) {
			//importants="<div style='color:#800;font-weight:bold;'>This match is important for "+players.pshorthtml(pids[0])+" and "+players.pshorthtml(pids[1])+"!</div>";
		}
		//ret+=importants;
		for(uint i=0;i<pids.size();i++) {
			auto &p=pids[i];
			if (players.players[p].headband) {
				headbands += ""+players.pshorthtml(p) + " has the #" + ToString((uint)players.players[p].headband) + " Headband. ";
			}
			string s=PlayerSection(p);
			//if(globalcontext.r==Renderer::bbcode) s="-----------------------"+s;
			if(globalcontext.r==Renderer::bbcode && i+1==pids.size()) s+="<br/>";
			if(i%2==0) retl+=s;
			else retr+=s;
		}
		if (headbands.length()) headbands = headbands.substr(0, headbands.length() - 1);
		string classes="upm";
		if(important) classes+=" important";
		//ret=LeftRight(retl,retr);
		if(globalcontext.r==Renderer::bbcode)
			ret="<table class='left-right-table'><tbody><tr><td class='ltd'>"+retl+"</td><td class='vs'></td><td class='rtd'>"+retr+"</td></tr></tbody></table>\n";
		else
			ret="<table class='left-right-table'><tbody><tr><td class='ltd'>"+retl+"</td><td class='vs'>VS</td><td class='rtd'>"+retr+"</td></tr></tbody></table>\n";

		string sround=RoundOf();
		//if(globalcontext.r==Renderer::bbcode)
		//	ret+="<br/>";
		string style = "";
		if (important) {
			int bw = std::max<int>(1,(int)((score-0.5)*12));
			bw = std::min<int>(10, bw);
			style = "border-right-width:" + ToString(bw) + "px; border-left-width:" + ToString(bw) + "px;";
		}
		return SubSection(h,"<mdiv><div style='white-space:nowrap;'>in "+tournaments.thtml(t_id)+" "+sround+".</div>"+importants+headbands+"</mdiv>"+ret, classes, style);
	}
};
class UpcomingTournament
{
public:
	uint t_id;
	uint timestamp;
	vector<WebEventGroup> evgs1;
	vector<WebEventGroup> evgs2;

	UpcomingTournament(uint TID)
	{
		t_id=TID;
		tournaments.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		timestamp=tournaments.tournies[t_id].timestamp;
		for(auto &e : events.evgs) {
			if(e.size()!=1) continue;
			if(e.events[0].t_id!=t_id) continue;
			if(e.type!=NORMAL_EVG_TYPE) continue;
			if (e.pid != e.events[0].p_id) continue;
			if(e.events[0].placing==1) evgs1.push_back(e);
			if(e.events[0].placing==2) evgs2.push_back(e);
		}
		std::sort(evgs1.begin(), evgs1.end(), [](const WebEventGroup &a, const WebEventGroup &b) {
			return a.hits+a.misses>b.hits+b.misses;
		});
		std::sort(evgs2.begin(), evgs2.end(), [](const WebEventGroup &a, const WebEventGroup &b) {
			return a.hits+a.misses>b.hits+b.misses;
		});
	}

	bool MustWin(WebEventGroup win)
	{
		uint samps = simheader.samples;
		WebPlayer &p = players.players[win.pid];
		WebEventGroup lose = win;
		lose.hits = p.times_top - win.hits;
		lose.misses = samps - (win.hits + win.misses);
		lose.type = NEG_EVG_TYPE;

		if (win.hits + win.misses == 0 || lose.hits + lose.misses == 0) return false;//if it can't find either event then give up
		double when_win = (double)win.hits / (double)(win.hits + win.misses) *100.0;
		double when_lose = (double)lose.hits / (double)(lose.hits + lose.misses) *100.0;
		if (when_win < 20.0) return false;
		if (when_win >(when_lose * 20)) return true;
		return false;
	}

	string MustWinsText()
	{
		if (t_id == 198) {
			User die4ever;
			die4ever.user_id = 1;
			die4ever.username = "Die4Ever";
			return "<mdiv class='must-wins'>"+die4ever.namehtml()+" must win this tournament!</mdiv>";
		}
		string ret;
		vector<uint> must_win_pids;
		for (auto e : evgs1) {
			if (MustWin(e)) must_win_pids.push_back(e.pid);
		}
		if (must_win_pids.size() == 0) return "";
		if (must_win_pids.size() > 20) must_win_pids.resize(20);

		for (uint p = 0; p<must_win_pids.size(); p++) {
			if (must_win_pids.size()>1 && p + 1 == must_win_pids.size()) ret += "and ";
			ret += players.pshorthtml(must_win_pids[p]);
			if (p + 1<must_win_pids.size() && must_win_pids.size()>2) ret += ", ";
			else if (p + 1<must_win_pids.size()) ret += " ";
		}
		ret += " must win this tournament!";
		return "<mdiv class='must-wins'>" + ret + "</mdiv>";
	}

	string HtmlLine(WebEventGroup e, WebEventGroup e2, uint samps)
	{
		string ret;
		auto &p=players.players[ e.pid ];
		double currchances=((double)p.times_top/(double)samps)*100.0;
		//double gains=(((double)e.hits/(double)(e.hits+e.misses))-((double)p.times_top/(double)samps)) *100.0;
		//ret+="<div class='tournament-winner'>"+players.phtml(e.pid)+" would gain "+RenderPercentD(gains, 0.0)+" if they win, with a "+RenderPercent(e.hits+e.misses, samps, 10.0)+" chance to win, going from "+RenderPercent(p.times_top, samps, players.cutoff)+" to "+RenderPercent(e.hits, e.hits+e.misses, players.cutoff)+" if they get 1st";
		if(globalcontext.r==Renderer::bbcode) {
			ret+="<mdiv class='tournament-winner'>"+players.phtml(e.pid)+" has a "+RenderPercent(e.hits+e.misses, samps, 10.0)+" chance to win<br/>----going from "+RenderPercent(p.times_top, samps, players.cutoff)+" to "+RenderPercent(e.hits, e.hits+e.misses, currchances)+" if they get 1st";
			if(e2.hits+e2.misses>0) ret+=", or "+RenderPercent(e2.hits, e2.hits+e2.misses, currchances)+" if they get 2nd";
			else if(e.hits+e.misses<samps) ret+=", or "+RenderPercent(p.times_top-e.hits, samps-(e.hits+e.misses), currchances)+" if they don't";
			ret += ".</mdiv>";
			return ret;
		}
		ret+="<mdiv class='tournament-winner'>"+players.phtml(e.pid)+" has a "+RenderPercent(e.hits+e.misses, samps, 10.0)+" chance to win, going from "+RenderPercent(p.times_top, samps, players.cutoff)+" to "+RenderPercent(e.hits, e.hits+e.misses, currchances)+" if they get 1st";
		if(e2.hits+e2.misses>0) ret+=", or "+RenderPercent(e2.hits, e2.hits+e2.misses, currchances)+" if they get 2nd";
		else if (e.hits + e.misses<samps)  ret += ", or " + RenderPercent(p.times_top - e.hits, samps - (e.hits + e.misses), currchances) + " if they don't";
		ret += ".</mdiv>";
		return ret;
	}

	string Body(uint num, uint pid DEBUGARGSDEF)
	{
		string ret;
		uint samps = simheader.samples;
		ShowMore showmore;
		uint outted = 0;
		if (pid) {
			for (auto &e : evgs1) {
				if (e.pid != pid) continue;
				WebEventGroup e2;
				/*for(uint b=0;b<evgs2.size();b++) {
				if(evgs2[b].pid==e.pid) {
				e2=evgs2[b];
				break;
				}
				}*/
				ret += HtmlLine(e, e2, samps);
				outted++;
				break;
			}
		}
		//return "";
		for (uint i = 0; i<evgs1.size() && outted<num; i++) {
			auto e = evgs1[i];
			if (e.pid == pid) continue;
			if (outted == 5) ret += showmore.Start("Winning Chances");
			WebEventGroup e2;
			/*for(uint b=0;b<evgs2.size();b++) {
			if(evgs2[b].pid==e.pid) {
			e2=evgs2[b];
			break;
			}
			}*/
			ret += HtmlLine(e, e2, samps);
			outted++;
		}
		if ((uint)evgs1.size()>outted) ret += "<mdiv>And " + ToString((uint)evgs1.size() - outted) + " more probable winners.</mdiv>";
		ret += showmore.End();
		return ret;
	}

	string Html(uint num DEBUGARGSDEF)
	{
		string ret = Body(num, 0 DEBUGARGS);
		string countdown;
		if(timestamp==0) countdown = "(unscheduled)";
		else countdown = CountdownHtml(timestamp, true);
		if (globalcontext.r == Renderer::bbcode) ret += "<br/>";
		ret=SubSection(tournaments.thtml(t_id)+" "+countdown, MustWinsText()+ret, "tournament-winners");
		return ret;
	}

	string HtmlPlayer(uint pid, uint num DEBUGARGSDEF)
	{
		ShowMore showmore;
		string ret = Body(num, pid DEBUGARGS);
		//uint samps=simheader.samples;
		string countdown;
		if(timestamp==0) countdown = "(unscheduled)";
		else countdown = CountdownHtml(timestamp, true);
		if (globalcontext.r == Renderer::bbcode) ret += "<br/>";
		ret=SubSection(tournaments.thtml(t_id)+" "+countdown, MustWinsText()+ret, "tournament-winners");
		return ret;
	}
};

class UpcomingMatches : public Module
{
public:
	vector<UpcomingMatchMod> matches;
	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		headbands.GetData(DEBUGARG);

		auto *res = mysqlcon.select(("select t_id, round, upm_id, group_concat(p_id), unix_timestamp(played_on) from upcoming_matches where sim_id="+ToString(simheader.sim_id)+" group by upm_id").c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			UpcomingMatchMod upm;
			auto row = mysqlcon.fetch_row(res);
			upm.t_id = row[0];
			upm.round = row[1];
			upm.upm_id = row[2];
			upm.timestamp = row[4];
			upm.estimated_time = false;
			upm.important = false;

			vector<string> splayers;
			SplitBy(row[3].cstr(), ',', splayers);
			for (uint p = 0; p < splayers.size(); p++) {
				upm.pids.push_back( ToUInt(splayers[p].c_str()) );
			}
			double score=0.0;
			for(auto e : events.evgs) {
				if (e.type != NORMAL_EVG_TYPE) continue;
				if (e.size() != 1) continue;
				if (e.pid != e.events[0].p_id) continue;
				if( (e.events[0].placing==UPCOMING_WIN || e.events[0].placing==UPCOMING_LOSE) && e.events[0].t_id==upm.upm_id) {
					upm.evgs.push_back(e);
					double prob=(double)players.players[e.pid].times_top/(double)simheader.samples;
					score=max(score, e.ScoreEvent(simheader.samples, prob, e.pid));
				}
			}
			if(score>0.8) upm.important=true;
			upm.score = score;
			for (auto p : upm.pids) {
				if (upm.MustWin(p)) upm.important = true;
			}
			matches.push_back(upm);
		}

		stable_sort(matches.begin(), matches.end(), [](const UpcomingMatchMod &a, const UpcomingMatchMod &b) {
			if (a.timestamp == b.timestamp) {
				return a.t_id < b.t_id;
			}
			if(a.timestamp==0) return false;
			if(b.timestamp==0) return true;
			return a.timestamp<b.timestamp;
		});
		if (matches.size()>100)
			matches.resize(100);
	}

	virtual string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for(uint i=0;i<matches.size();i++) {
			if(i==4) ret+=showmore.Start("Upcoming Matches");
			auto &m=matches[i];
			ret+=m.Html(DEBUGARG);
		}
		ret+=showmore.End();
		if(matches.size()==0) {
			ret+="<div style='text-align:left;'>No upcoming matches</div>";
		}
		return ret;
	}

	virtual string HtmlPlayer(uint pid DEBUGARGSDEF)
	{
		GetData(DEBUGARG);

		string ret;
		ShowMore showmore;
		uint count=0;
		for(uint i=0;i<matches.size();i++) {
			bool found=false;
			for(auto p : matches[i].pids) {
				if(p==pid) found=true;
			}
			if(found==false) continue;

			if(count==4) ret+=showmore.Start("Upcoming Matches");
			auto &m=matches[i];
			ret+=m.Html(DEBUGARG);
			count++;
		}
		ret+=showmore.End();
		if(count==0) {
			ret+="<div style='text-align:left;'>No upcoming matches</div>";
		}
		return ret;
	}
};
UpcomingMatches upcomingmatches;

class UpcomingTournaments : public Module
{
public:
	vector<UpcomingTournament> tournies;
	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		upcomingmatches.GetData(DEBUGARG);

		for(auto &t : tournaments.tournies) {
			//if(t.timestamp==0) continue;
			if(t.timestamp && t.timestamp+86400*10 < (uint)time(0)) continue;
			if(t.name.length()==0) continue;
			UpcomingTournament ut(t.id);
			ut.timestamp=t.timestamp;
			for(auto &m : upcomingmatches.matches) {
				if(m.t_id==t.id && m.timestamp>0 && (m.timestamp<ut.timestamp || ut.timestamp==0) ) {
					ut.timestamp=m.timestamp;
				}
			}
			tournies.push_back(ut);
		}
		std::stable_sort(tournies.begin(), tournies.end(), [](const UpcomingTournament &a, const UpcomingTournament &b) {
			if(a.timestamp==0) return false;
			if(b.timestamp==0) return true;
			return a.timestamp<b.timestamp;
		});
	}

	virtual string InnerHtml()
	{
		string ret;
		ShowMore showmore;
		for(uint i=0;i<tournies.size();i++) {
			auto &t=tournies[i];
			if(i==2) ret+=showmore.Start("Upcoming Tournaments");
			ret+=t.Html(5 DEBUGARGS);
		}
		ret+=showmore.End();
		return ret;
	}
};
UpcomingTournaments upcomingtournaments;

class Upcoming : public Module
{
public:
	virtual void RunQuery()
	{
		upcomingmatches.GetData(DEBUGARG);
		upcomingtournaments.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		vector<std::pair<uint,string> > ups_htmls;
		//array<bool,1025> tournies_has_upms = {{false}};
		string ret;
		ShowMore showmore;
		for(uint i=0;i<upcomingtournaments.tournies.size();i++) {
			auto &t=upcomingtournaments.tournies[i];
			//if(t.t_id>=tournies_has_upms.size()) continue;
			//if(t.timestamp==0) continue;
			//if( t.timestamp+86400*10 < (uint)time(0)) continue;
			//if(t.evgs1.size()==0) continue;
			//if(tournies_has_upms[t.t_id]==true) continue;
			if (t.timestamp == 0 && t.evgs1.size() == 0) continue;
			string s=t.Html(4 DEBUGARGS);
			ups_htmls.push_back( make_pair(t.timestamp,s) );
		}
		for(uint i=0;i<upcomingmatches.matches.size();i++) {
			//if(i==4) ret+=showmore.Start("Upcoming");
			auto &m=upcomingmatches.matches[i];
			string s=m.Html(DEBUGARG);
			ups_htmls.push_back( make_pair(m.timestamp,s) );
			//if(m.t_id<tournies_has_upms.size()) tournies_has_upms[m.t_id]=true;
		}
		stable_sort(ups_htmls.begin(), ups_htmls.end(), [](const std::pair<uint,string> &a, const std::pair<uint,string> &b) {
			if(a.first==0) return false;
			if(b.first==0) return true;
			return a.first<b.first;
		});
		if (ups_htmls.size()>100)
			ups_htmls.resize(100);
		for(uint i=0;i<ups_htmls.size();i++) {
			if(i==4) ret+=showmore.Start("Upcoming");
			ret+=ups_htmls[i].second;
		}
		ret+=showmore.End();
		if(ups_htmls.size()==0) {
			ret+="<div style='text-align:left;'>No upcoming matches or tournaments</div>";
		}
		return ret;
	}

	virtual string HtmlPlayer(uint pid DEBUGARGSDEF)
	{
		GetData(DEBUGARG);
		vector<std::pair<uint,string> > ups_htmls;
		//array<bool,1025> tournies_has_upms = {{false}};
		string ret;
		ShowMore showmore;
		for(uint i=0;i<upcomingtournaments.tournies.size();i++) {
			auto &t=upcomingtournaments.tournies[i];
			//if(t.t_id>=tournies_has_upms.size()) continue;
			//if(t.timestamp==0) continue;
			//if(tournies_has_upms[t.t_id]==true) continue;
			bool has_player=false;
			for(auto &e : t.evgs1) {
				if(e.pid==pid) has_player=true;
			}
			if(has_player==false) continue;
			string s=t.HtmlPlayer(pid, 4 DEBUGARGS);
			ups_htmls.push_back( make_pair(t.timestamp,s) );
		}
		for(uint i=0;i<upcomingmatches.matches.size();i++) {
			//if(i==4) ret+=showmore.Start("Upcoming");
			auto &m=upcomingmatches.matches[i];
			bool has_player=false;
			for(auto p : m.pids) {
				if(p==pid) has_player=true;
			}
			if(has_player==false) continue;
			string s=m.Html(DEBUGARG);
			ups_htmls.push_back( make_pair(m.timestamp,s) );
			//if(m.t_id<tournies_has_upms.size()) tournies_has_upms[m.t_id]=true;
		}
		stable_sort(ups_htmls.begin(), ups_htmls.end(), [](const std::pair<uint,string> &a, const std::pair<uint,string> &b) {
			if(a.first==0) return false;
			if(b.first==0) return true;
			return a.first<b.first;
		});
		if (ups_htmls.size()>100)
			ups_htmls.resize(100);
		for(uint i=0;i<ups_htmls.size();i++) {
			if(i==4) ret+=showmore.Start("Upcoming");
			ret+=ups_htmls[i].second;
		}
		ret+=showmore.End();
		if(ups_htmls.size()==0) {
			ret+="<div style='text-align:left;'>No upcoming matches or tournaments</div>";
		}
		return ret;
	}
};
Upcoming upcoming;

class TournamentUpcoming : public Module
{
public:
	uint tid;
	UpcomingTournaments upts;
	UpcomingMatches upms;

	TournamentUpcoming(uint TID)
	{
		tid=TID;
	}

	virtual void RunQuery()
	{
		upcoming.GetData(DEBUGARG);
		upts=upcomingtournaments;
		upms=upcomingmatches;
		upts.tournies.clear();
		upms.matches.clear();
		for(auto &m : upcomingmatches.matches) {
			if(m.t_id==tid) upms.matches.push_back(m);
		}
		for(auto &t : upcomingtournaments.tournies) {
			if(t.t_id==tid) upts.tournies.push_back(t);
		}
	}

	virtual string InnerHtml()
	{
		vector<std::pair<uint,string> > ups_htmls;
		//array<bool,1025> tournies_has_upms = {{false}};
		string ret;
		ShowMore showmore;
		for(uint i=0;i<upts.tournies.size();i++) {
			//auto &t=upts.tournies[i];
			//if(t.t_id>=tournies_has_upms.size()) continue;
			//if(t.timestamp==0) continue;
			//if(tournies_has_upms[t.t_id]==true) continue;
			//string s=t.Html(100 DEBUGARGS);//I don't have to worry about the double show/hide because this will only show if there are no matches
			//ups_htmls.push_back( make_pair(t.timestamp,s) );
		}
		for(uint i=0;i<upms.matches.size();i++) {
			auto &m=upms.matches[i];
			string s=m.Html(DEBUGARG);
			ups_htmls.push_back( make_pair(m.timestamp,s) );
			//if(m.t_id<tournies_has_upms.size()) tournies_has_upms[m.t_id]=true;
		}
		stable_sort(ups_htmls.begin(), ups_htmls.end(), [](const std::pair<uint,string> &a, const std::pair<uint,string> &b) {
			if(a.first==0) return false;
			if(b.first==0) return true;
			return a.first<b.first;
		});
		if (ups_htmls.size()>100)
			ups_htmls.resize(100);
		for(uint i=0;i<ups_htmls.size();i++) {
			if(i==4) ret+=showmore.Start("Upcoming");
			ret+=ups_htmls[i].second;
		}
		ret+=showmore.End();
		if(ups_htmls.size()==0) {
			ret+="<div style='text-align:left;'>No upcoming matches or tournaments</div>";
		}
		return ret;
	}
};

class PlayerSetUpcoming : public Module
{
public:
	vector<uint> pids;
	UpcomingTournaments upts;
	UpcomingMatches upms;

	PlayerSetUpcoming(vector<uint> &PIDs) : pids(PIDs)
	{
	}

	virtual void RunQuery()
	{
		vector<uint> pid_map(MAX_PLAYER_ID);
		for(auto &p : pid_map) p=0;
		for(auto &p : pids) pid_map[p]=p;

		upcoming.GetData(DEBUGARG);
		upts=upcomingtournaments;
		upms=upcomingmatches;
		upts.tournies.clear();
		upms.matches.clear();
		for(auto &m : upcomingmatches.matches) {
			bool found=false;
			for(auto p : m.pids) {
				if(pid_map[p]) found=true;
			}
			if(found) upms.matches.push_back(m);
		}
		for(auto &t : upcomingtournaments.tournies) {
			bool found=false;
			for(auto &e : t.evgs1) {
				if(pid_map[e.pid]) found=true;
			}
			if(found) upts.tournies.push_back(t);
		}
		for(auto &t : upts.tournies) {
			auto evgs1=t.evgs1;
			auto evgs2=t.evgs2;
			t.evgs1.clear();
			t.evgs2.clear();
			for(auto &e : evgs1) {
				if(pid_map[e.pid]) t.evgs1.push_back(e);
			}
			for(auto &e : evgs2) {
				if(pid_map[e.pid]) t.evgs2.push_back(e);
			}
		}
	}

	virtual string InnerHtml()
	{
		vector<std::pair<uint,string> > ups_htmls;
		//array<bool,1025> tournies_has_upms = {{false}};
		string ret;
		ShowMore showmore;
		for(uint i=0;i<upts.tournies.size();i++) {
			auto &t=upts.tournies[i];
			//if(t.t_id>=tournies_has_upms.size()) continue;
			//if(t.timestamp==0) continue;
			//if(tournies_has_upms[t.t_id]==true) continue;
			string s=t.Html(100 DEBUGARGS);//I don't have to worry about the double show/hide because this will only show if there are no matches
			ups_htmls.push_back( make_pair(t.timestamp,s) );
		}
		for(uint i=0;i<upms.matches.size();i++) {
			auto &m=upms.matches[i];
			string s=m.Html(DEBUGARG);
			ups_htmls.push_back( make_pair(m.timestamp,s) );
			//if(m.t_id<tournies_has_upms.size()) tournies_has_upms[m.t_id]=true;
		}
		stable_sort(ups_htmls.begin(), ups_htmls.end(), [](const std::pair<uint,string> &a, const std::pair<uint,string> &b) {
			if(a.first==0) return false;
			if(b.first==0) return true;
			return a.first<b.first;
		});
		if (ups_htmls.size()>100)
			ups_htmls.resize(100);
		for(uint i=0;i<ups_htmls.size();i++) {
			if(i==4) ret+=showmore.Start("Upcoming");
			ret+=ups_htmls[i].second;
		}
		ret+=showmore.End();
		if(ups_htmls.size()==0) {
			ret+="<div style='text-align:left;'>No upcoming matches or tournaments</div>";
		}
		return ret;
	}
};

string EventToHtml(WebEventGroup &evg)
{
	string ret;// = "<mdiv>";
	uint pid = evg.pid;
	uint samps = simheader.samples;
	double currchances = (double)players.players[pid].times_top / (double)samps;
	double newchances = (double)evg.hits / (double)(evg.hits + evg.misses);
	double score = evg.ScoreEvent(simheader.samples, currchances, pid);
	uint type = evg.type;
	if (type != NORMAL_EVG_TYPE) score *= 0.25;
	if (type == NEG_AND_EVG_TYPE || type == NEG_EVG_TYPE) score *= 0.25;

	string chances_to;
	if (newchances < currchances) chances_to = " down";
	if (newchances > currchances) chances_to = " up";

	for (uint i = 0; i < evg.size(); i++) {
		auto &e = evg.events[i];
		uint placing = e.placing;
		uint tid = (uint)e.t_id;
		uint tpid = e.p_id;
		if (i>0 && i + 1 == evg.size()) ret += "and ";
		if (placing < SPECIAL_EVENT) {
			ret += players.pshorthtml(tpid) + " gets " + IntToPlacing(placing) + " in " + tournaments.thtml(tid) + ", ";
		}
		else if (placing == UPCOMING_WIN || placing == UPCOMING_LOSE) {
			string against;
			string roundof;
			for (auto &m : upcomingmatches.matches) {
				if (m.upm_id == tid) {
					tid = m.t_id;
					roundof = m.RoundOf();
					for (auto p : m.pids) {
						if (p == 0) continue;
						against += players.players[p].name.ToString() + ",";
					}
					break;
				}
			}
			string wins_loses = placing == UPCOMING_WIN ? " wins" : " loses";
			ret += players.pshorthtml(tpid) + wins_loses+ " their next match in the " + tournaments.thtml(tid) + " " + roundof + ", ";
		}
		else if (placing == QUALIFY_BLIZZCON) {
			ret += players.pshorthtml(tpid) + " qualifies for Blizzcon, ";
		}
	}
	if (evg.size() > 0) ret = ret.substr(0, ret.length() - 2);
	ret += ". This would bring " + players.pshorthtml(pid) + chances_to + " to " + RenderPercent(evg.hits, evg.hits + evg.misses)+".";
	return ret;
}

string EventToJson(WebEventGroup &evg)
{
	uint pid = evg.pid;
	uint samps = simheader.samples;
	double currchances = (double)players.players[pid].times_top / (double)samps;
	double score = evg.ScoreEvent(simheader.samples, currchances, pid);
	uint type = evg.type;
	if (type != NORMAL_EVG_TYPE) score *= 0.25;
	if (type == NEG_AND_EVG_TYPE || type == NEG_EVG_TYPE) score *= 0.25;
	string events_json;

	for (uint i = 0; i < evg.size(); i++) {
		auto &e = evg.events[i];
		uint placing = e.placing;
		uint tid = (uint)e.t_id;
		uint tpid = e.p_id;
		if (placing < SPECIAL_EVENT) {
			events_json += "[" + ToString(tpid) + ",'" + jsonescape(players.players[tpid].name.ToString(), 256) + "'," + ToString(placing) + "," + ToString(tid) + ",'" + jsonescape(tournaments.tournies[tid].name, 256) + "'],";
		} else if (placing == UPCOMING_WIN || placing == UPCOMING_LOSE) {
			string against;
			uint roundof = 16;
			for (auto &m : upcomingmatches.matches) {
				if (m.upm_id == tid) {
					tid = m.t_id;
					roundof = m.round;
					for (auto p : m.pids) {
						if (p == 0) continue;
						against += players.players[p].name.ToString() + ",";
					}
					break;
				}
			}
			events_json += "[" + ToString(tpid) + ",'" + jsonescape(players.players[tpid].name.ToString(), 256) + "'," + ToString(placing) + "," + ToString(tid) + ",'" + jsonescape(tournaments.tournies[tid].name, 256) + "'," + ToString(roundof) + ",'" + jsonescape(against, 256) + "'],";
		}
		else if (placing == QUALIFY_BLIZZCON) {
			events_json += "[" + ToString(tpid) + ",'" + jsonescape(players.players[tpid].name.ToString(), 256) + "'," + ToString(placing) + "," + ToString(tid) + ",'Blizzcon'],";
		}
	}
	if (events_json.length() > 0) events_json = events_json.substr(0, events_json.length() - 1);
	string ret = "[" + ToString(pid) + ",'" + jsonescape(players.players[pid].name.ToString(), 256) + "'," + std::to_string(score) + "," + PercentToJson(evg.hits + evg.misses, samps) + "," + PercentToJson(players.players[pid].times_top, samps) + "," + PercentToJson(evg.hits, evg.hits + evg.misses) + "," + ToString(type) + ",[" + events_json + "]]";
	return ret;
}

class PlayerEvents : public Module
{
public:
	uint pid;
	vector<WebEventGroup> evgs;

	PlayerEvents(uint PID)
	{
		pid=PID;
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		upcomingmatches.GetData(DEBUGARG);
		for(auto &evg : events.evgs) {
			if(evg.pid==pid) {
				evgs.push_back(evg);
			}
		}
	}

	virtual string InnerHtml()
	{
		string ret="<table class='events-table'></table>\n";
		ret+="<script>window.wcs.eventsa=[";
		for(auto &evg : evgs) {
			ret+=EventToJson(evg)+",";
		}
		if(evgs.size()>0) {
			ret=ret.substr(0,ret.length()-1);
		}
		ret+="];</script>\n";
		return ret;
	}
};

class TournamentEvents : public Module
{
public:
	uint tid;
	vector<WebEventGroup> evgs;

	TournamentEvents(uint TID)
	{
		tid = TID;
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		upcomingmatches.GetData(DEBUGARG);
		for (auto &evg : events.evgs) {
			for (uint i = 0; i < evg.size(); i++) {
				auto &e = evg.events[i];
				if (e.placing == UPCOMING_WIN || e.placing == UPCOMING_LOSE) {
					uint etid = 0;
					for (auto &u : upcomingmatches.matches) {
						if (u.upm_id==e.t_id) {
							etid = u.t_id;
							break;
						}
					}
					if (etid == tid) {
						evgs.push_back(evg);
						break;
					}
				}
				else if (e.t_id == tid) {
					evgs.push_back(evg);
					break;
				}
			}
		}
	}

	virtual string InnerHtml()
	{
		string ret = "<table class='events-table'></table>\n";
		ret += "<script>window.wcs.eventsa=[";
		for (auto &evg : evgs) {
			ret += EventToJson(evg) + ",";
		}
		if (evgs.size()>0) {
			ret = ret.substr(0, ret.length() - 1);
		}
		ret += "];</script>\n";
		return ret;
	}
};

class PlayerSetEvents : public Module
{
public:
	vector<uint> pids;
	vector<WebEventGroup> evgs;

	PlayerSetEvents(vector<uint> &PIDs) : pids(PIDs)
	{
	}

	virtual void RunQuery()
	{
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		upcomingmatches.GetData(DEBUGARG);

		array<bool, MAX_PLAYER_ID> mask = { { 0 } };
		for (auto p : pids) mask[p] = true;

		for (auto &evg : events.evgs) {
			if (mask[evg.pid]) {
				evgs.push_back(evg);
			}
		}
	}

	virtual string InnerHtml()
	{
		string ret = "<table class='events-table'></table>\n";
		ret += "<script>window.wcs.eventsa=[";
		for (auto &evg : evgs) {
			ret += EventToJson(evg) + ",";
		}
		if (evgs.size()>0) {
			ret = ret.substr(0, ret.length() - 1);
		}
		ret += "];</script>\n";
		return ret;
	}
};

class ForeignerHopes : public Module
{
public:
	vector< std::pair<uint,uint> > hopes;
	//array<uint,17> num_foreigners;
	string histories;
	
	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		auto * res = mysqlcon.select(("select p_id,num_times_only from foreigner_hopes where sim_id="+ToString(simheader.sim_id)+" order by num_times_only desc limit 100").c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint times_only = row[1];
			hopes.push_back( std::make_pair(pid, times_only) );
		}
	
		//"select players_chances.p_id, times_top, players_chances.sim_id, sims.size, unix_timestamp(sims.created), min, median, mean, afk_times_top, afk_samps from players_chances join sims using(sim_id) join players_wcs_points on (players_wcs_points.sim_id=sims.sim_id and players_wcs_points.p_id=players_chances.p_id) where players_chances.p_id in("+spids+") and sims.created between from_unixtime("+ToString(start)+")-interval 1 second and from_unixtime("+ToString(end)+")+interval 1 second and (sims.size>="+ToString(HISTORY_MIN_SIZE)+" or players_chances.sim_id=" + ToString(simheader.sim_id) + " or players_chances.sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac="+ToString(USE_ALIGULAC)+" order by players_chances.p_id,created"
		uint start=simheader.comp_created;
		uint end=simheader.created;
		res = mysqlcon.select(("select num_foreigners,num_times,unix_timestamp(created),sim_id,size from num_foreigner_hopes join sims using(sim_id) where sims.created>'2014-11-01' and sims.created between from_unixtime(" + ToString(start) + ")-interval 1 second and from_unixtime(" + ToString(end) + ")+interval 1 second and (sims.size>=" + ToString(HISTORY_MIN_SIZE) + " or sim_id=" + ToString(simheader.sim_id) + " or sim_id=" + ToString(simheader.comp_sim_id) + ") and use_aligulac=" + ToString(USE_ALIGULAC) + " and (sims.type=" + ToString(SIM_TYPE) + " or sim_id=" + ToString(simheader.sim_id) + " or sim_id="+ToString(simheader.comp_sim_id)+") order by created,num_foreigners").c_str() DEBUGARGS);
		num_rows = mysql_num_rows(res);
		histories="";
		uint last_sim_id=0;
		uint last_created=0;
		uint last_samps=0;
		array<uint,8> num_foreigners;
		for(auto &t : num_foreigners) t=0;
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint num = row[0];
			uint times = row[1];
			uint created = row[2];
			uint sim_id = row[3];
			uint samps = row[4];
			if(sim_id!=last_sim_id && last_sim_id>0) {
				if(histories.length()) histories+=",";
				histories+="["+ToString(last_created)+","+ToString(last_samps);
				for(auto &t : num_foreigners) {
					histories+=","+ToString(t);
					t=0;
				}
				histories+="]";
			}
			last_sim_id=sim_id;
			last_created=created;
			last_samps=samps;
			for(uint i=min(num,8u)-1;i<100;i--) {
				num_foreigners[i]+=times;
			}
		}
		if(last_sim_id && last_sim_id>0) {
			if(histories.length()) histories+=",";
			histories+="["+ToString(last_created)+","+ToString(last_samps);
			for(auto &t : num_foreigners) {
				histories+=","+ToString(t);
				t=0;
			}
			histories+="]";
		}
	}
	
	virtual string InnerHtml()
	{
		uint samps=simheader.samples;
		string lret;
		ShowMore showmore;
		for(uint i=0;i<hopes.size();i++) {
			auto &f=hopes[i];
			if(i==7)	lret+=showmore.Start("Foreigner Hopes");
			lret+="<mdiv>"+players.phtml(f.first)+" "+RenderPercent(f.second,samps)+" chance to be the only foreigner, "+RenderPercent(players.players[f.first].times_top, samps, players.cutoff)+" chance overall.</mdiv>";
		}
		lret+=showmore.End();
		string rret="<div class='graph numforeigners-history-graph'></div>";
		return LeftRight(lret, rret)+"\n<script>window.wcs.num_foreigner_hopes.push("+histories+");</script>\n";
	}
};
ForeignerHopes foreigner_hopes;

class Country
{
public:
	//string name;
	string abrv;
	uint times_top;
	uint comp_times_top;
	
	Country(string Abrv)
	{
		/*name=*/abrv=Abrv;
		times_top=0;
		comp_times_top=0;
	}
	
	static unsigned int Hash(const char *k)
	{
		unsigned int h=0;
		for(;*k;k++)
			h=h*HASHPRIME+*k;
		return h;
	}
	
	unsigned int Hash()
	{
		return Hash(abrv.c_str());
	}
	
	int SortComp(Country* objB)
	{
		return abrv != objB->abrv;
	}
	
	int SortComp(const char *objB)
	{
		return abrv != objB;
	}
};

class SummedPercentages : public Module {
public:
	uint races_times_tops[4];
	uint comp_races_times_tops[4];
	HashTable<Country,const char*> countries;
	
	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		for(auto &r : races_times_tops) r=0;
		for(auto &r : comp_races_times_tops) r=0;

		for(auto &p : players.top_players) {
			races_times_tops[(int)p.race]+=p.times_top;
			comp_races_times_tops[(int)p.race]+=p.comp_times_top;
			if(p.times_top==0) continue;
			string country=p.sCountry();
			if(country.length()==0) country="none";
			auto * pc = countries.Get(country.c_str());
			if(!pc) {
				Country c(country);
				pc = countries.Create( c );
				pc->times_top = p.times_top;
				pc->comp_times_top += p.comp_times_top;
			} else {
				pc->times_top += p.times_top;
				pc->comp_times_top += p.comp_times_top;
			}
		}
	}
	
	virtual string InnerHtml()
	{
		string ret;
		ret+="<script>\n";
		ret+="window.wcs.races={'protoss':{'times_top':"+ToString(races_times_tops[(int)P])+",'comp_times_top':"+ToString(comp_races_times_tops[(int)P])+"}, 'terran':{'times_top':"+ToString(races_times_tops[(int)T])+",'comp_times_top':"+ToString(comp_races_times_tops[(int)T])+"}, 'zerg':{'times_top':"+ToString(races_times_tops[(int)Z])+",'comp_times_top':"+ToString(comp_races_times_tops[(int)Z])+"}, 'random':{'times_top':"+ToString(races_times_tops[(int)R])+",'comp_times_top':"+ToString(comp_races_times_tops[(int)R])+"} };\n";
		/*string s_countries;
		for(auto *c=countries.GetFirst(); c; c=countries.GetNext(c)) {
			s_countries += "'"+c->abrv+"':{'times_top':"+ToString(c->times_top)+"},";
		}
		if(s_countries.length()) s_countries=s_countries.substr(0, s_countries.length()-1);
		ret+="window.wcs.countries={"+s_countries+"};\n";*/
		string s_teams;
		for(auto &t : players.teams) {
			if(t.times_top==0) continue;
			s_teams+="'"+ToString(t.team_id)+"':{'name':'"+jsonescape(t.name.ToString(), 256)+"','times_top':"+ToString(t.times_top)+",'comp_times_top':"+ToString(t.comp_times_top)+"},";
		}
		if(s_teams.length()) s_teams=s_teams.substr(0, s_teams.length()-1);
		ret+="window.wcs.teams={"+s_teams+"};\n";
		ret+="</script>\n";
		ret+=LeftRight("<div class='graph races-graph'></div>", "<div class='graph teams-graph'></div>");
		return ret;
	}
	
	virtual string CountriesHtml(DEBUGARGDEF)
	{
		GetData(DEBUGARG);
		string ret;
		ret+="<script>\n";
		string s_countries;
		for(auto *c=countries.GetFirst(); c; c=countries.GetNext(c)) {
			s_countries += "'"+c->abrv+"':{'times_top':"+ToString(c->times_top)+",'comp_times_top':"+ToString(c->comp_times_top)+"},";
		}
		if(s_countries.length()) s_countries=s_countries.substr(0, s_countries.length()-1);
		ret+="window.wcs.countries={"+s_countries+"};\n";
		ret+="</script>\n";
		ret+="<div class='graph countries-graph'></div>";
		return ret;

	}
};
class SummedPercentages summedpercentages;

class GroupOfDeath : public Module
{
public:
	uint tid;
	TournamentUpcoming tu;

	GroupOfDeath(uint TID, TournamentUpcoming &TU) : tu(TU)
	{
		tid = TID;
	}

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
	}

	string Ranking(uint i, vector<string> &group_names, vector<array<double, 5> > &scores, vector<array<int, 4> > &placements)
	{
		vector< std::pair<uint, double> > score_sort;
		for (uint a = 0; a < scores.size();a++) {
			score_sort.push_back(std::make_pair(a, scores[a][i]));
		}
		std::stable_sort(score_sort.begin(), score_sort.end(), [](const std::pair<uint, double> &a, const std::pair<uint, double> &b) {
			return a.second > b.second;
		});
		string ret;
		for (uint a = 0; a < score_sort.size(); a++) {
			if (a < 4) placements[score_sort[a].first][a]++;
			string name = group_names[score_sort[a].first];
			name = str_replace(name.c_str(), "Group ", "");
			ret += name;
			if(a+1<score_sort.size()) ret+=" > ";
		}
		return ret;
	}

	virtual string InnerHtml()
	{
		vector<string> group_names;
		vector<array<double, 5> > scores;
		vector<array<int, 4> > placements;
		uint base_upm_id = 10000;
		auto &upms = tu.upms.matches;
		auto &ut = tu.upts.tournies[0];
		group_names.resize(upms.size());
		scores.resize(upms.size());
		placements.resize(upms.size());
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			base_upm_id = min(base_upm_id, u.upm_id);
		}
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			char g[2];
			g[1] = 0;
			g[0] = 'A' + (u.upm_id - base_upm_id);
			group_names[i] = "Group " + string(g);
		}

		string ret;
		ret += "And now let's do some analysis to see what the Group of Death is according to WCS Predictor.<br/>"
			"We will be looking at 5 different scores.<br/>"
			"Champion Chances Score is the chances to win the tournament for each player in the group added together.<br/>"
			"Blizzcon Chances Score is the chances to qualify for Blizzcon for each player in the group added together.<br/>"
			"Blizzcon Chances Lost Score is how much the Blizzcon Chances Score went down by because of the group drawing.<br/>"
			"Effects Score is the sum of the amount of Blizzcon chances each player would gain if they won the group, plus the amount of Blizzcon chances each player would lose if they lost the group.<br/>"
			"Aligulac Score is the average Aligulac rating of the players in the group.<br/><br/>";

		string champ_placings;
		string champs;
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			double score = 0.0;
			uint total = 0;
			if (globalcontext.r == Renderer::bbcode) champs += "[b]" + group_names[i] + "[/b]<br/>";
			else champs += group_names[i] + "<br/>";

			for (auto p : u.pids) {
				uint w = 0;
				for (auto &e : ut.evgs1) {
					if (e.pid == p) {
						w = (e.hits + e.misses);
						break;
					}
				}
				total += w;
				champs += players.pshorthtml(p) + " has a " + RenderPercent(w, simheader.samples) + " chance to win "+tournaments.tournies[ut.t_id].name+"<br/>";
			}
			score = (double)total / (double)simheader.samples *100.0;
			champs += "Giving this match a Champion Chances Score of "+ToString(score)+"<br/><br/>";
			scores[i][0] = score;
		}

		string blizz_placings;
		string blizzs;
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			double score = 0.0;
			uint total = 0;
			if (globalcontext.r == Renderer::bbcode) blizzs += "[b]" + group_names[i] + "[/b]<br/>";
			else blizzs += group_names[i] + "<br/>";

			for (auto p : u.pids) {
				uint w = players.players[p].times_top;
				total += w;
				blizzs += players.pshorthtml(p) + " has a " + RenderPercent(w, simheader.samples) + " chance to qualify for Blizzcon<br/>";
			}
			score = (double)total / (double)simheader.samples *100.0;
			blizzs += "Giving this match a Blizzcon Chances Score of " + ToString(score) + "<br/><br/>";
			scores[i][1] = score;
		}

		string blizz_lost_placings;
		string blizz_losts;
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			double score = 0.0;
			double total = 0.0;
			if (globalcontext.r == Renderer::bbcode) blizz_losts += "[b]" + group_names[i] + "[/b]<br/>";
			else blizz_losts += group_names[i] + "<br/>";

			for (auto p : u.pids) {
				double w = (double)players.players[p].times_top/(double)simheader.samples;
				w -= (double)players.players[p].comp_times_top / (double)simheader.comp_samples;
				w *= 100.0;
				total += w;
				blizz_losts += players.pshorthtml(p) + "'s Blizzcon Chances changed by " + RenderPercentD(w) + "<br/>";
			}
			score = total*-1.0;
			blizz_losts += "Giving this match a Blizzcon Chances Lost Score of " + ToString(score) + "<br/><br/>";
			scores[i][2] = score;
		}

		string effects_placings;
		string effects;
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			double score = 0.0;
			double total = 0.0;
			if (globalcontext.r == Renderer::bbcode) effects += "[b]" + group_names[i] + "[/b]<br/>";
			else effects += group_names[i] + "<br/>";

			for (auto p : u.pids) {
				double c = (double)players.players[p].times_top / (double)simheader.samples *100.0;
				double w = 0.0;
				double l = 0.0;
				for (auto e : u.evgs) {
					if (e.type != NORMAL_EVG_TYPE) continue;
					if (e.events[0].p_id == p && e.events[0].placing == UPCOMING_WIN) {
						w = (double)e.hits / (double)(e.hits+e.misses)*100.0;
					}
					if (e.events[0].p_id == p && e.events[0].placing == UPCOMING_LOSE) {
						l = (double)e.hits / (double)(e.hits+e.misses)*100.0;
					}
				}
				w -= c;
				l = c - l;
				total += w+l;
				effects += "When "+players.pshorthtml(p) + " wins this match, their Blizzcon Chances go up by " + RenderPercentD(w) + "<br/>";
				effects += "When " + players.pshorthtml(p) + " loses this match, their Blizzcon Chances go down by " + RenderPercentD(l) + "<br/>";
			}
			score = total;
			effects += "Giving this match an Effects Score of " + ToString(score) + "<br/><br/>";
			scores[i][3] = score;
		}

		string aligulacs_placings;
		string aligulacs;
		for (uint i = 0; i < upms.size(); i++) {
			auto &u = upms[i];
			double score = 0.0;
			int total = 0;
			if (globalcontext.r == Renderer::bbcode) aligulacs += "[b]" + group_names[i] + "[/b]<br/>";
			else aligulacs += group_names[i] + "<br/>";

			for (auto p : u.pids) {
				int w = players.players[p].ratings[(uint)R];
				total += w;
				aligulacs += players.pshorthtml(p) + " has an Aligulac rating of " + ToString(w) + "<br/>";
			}
			score = (double)total;
			aligulacs += "Giving this match an Aligulac Score of " + ToString(total) + "<br/><br/>";
			scores[i][4] = score;
		}

		champ_placings = Ranking(0, group_names, scores, placements);
		blizz_placings = Ranking(1, group_names, scores, placements);
		blizz_lost_placings = Ranking(2, group_names, scores, placements);
		effects_placings = Ranking(3, group_names, scores, placements);
		aligulacs_placings = Ranking(4, group_names, scores, placements);

		ret += SubSection("Champion Chances", champ_placings + "<br/>" + (globalcontext.r == Renderer::bbcode ? "[spoiler=Champion Chances]" : "") + champs + (globalcontext.r == Renderer::bbcode ? "[/spoiler]":""));
		ret += SubSection("Blizzcon Chances", blizz_placings + "<br/>" + (globalcontext.r == Renderer::bbcode ? "[spoiler=Blizzcon Chances]" : "") + blizzs + (globalcontext.r == Renderer::bbcode ? "[/spoiler]" : ""));
		ret += SubSection("Blizzcon Chances Lost", blizz_lost_placings + "<br/>" + (globalcontext.r == Renderer::bbcode ? "[spoiler=Blizzcon Chances Lost]" : "") + blizz_losts + (globalcontext.r == Renderer::bbcode ? "[/spoiler]" : ""));
		ret += SubSection("Effects", effects_placings + "<br/>" + (globalcontext.r == Renderer::bbcode ? "[spoiler=Effects]" : "") + effects + (globalcontext.r == Renderer::bbcode ? "[/spoiler]" : ""));
		ret += SubSection("Aliglac Ratings", aligulacs_placings + "<br/>" + (globalcontext.r == Renderer::bbcode ? "[spoiler=Aligulac Ratings]" : "") + aligulacs + (globalcontext.r == Renderer::bbcode ? "[/spoiler]" : ""));

		string final_placings;
		array<string, 11> numbers = { {"zero","one","two","three","four","five","six","seven","eight","nine","ten"} };
		for (uint i = 0; i < placements.size(); i++) {
			auto &p = placements[i];
			final_placings += group_names[i] + " has ";
			uint total = 0;
			for (auto a : p) {
				total += a;
			}
			if (total == 0) final_placings += " no top 4 placements";
			uint so_far = 0;
			if (p[0]) {
				so_far += (uint)p[0];
				final_placings += numbers[p[0]] + " 1st place" + (p[0] > 1 ? "s" : "") + (total>so_far ? ", ":"");
			}
			if (p[1]) {
				so_far += p[1];
				if (so_far == total && (int)total > p[1]) final_placings += "and ";
				final_placings += numbers[p[1]] + " 2nd place" + (p[1] > 1 ? "s" : "") + (total>so_far ? ", " : "");
			}
			if (p[2]) {
				so_far += p[2];
				if (so_far == total && (int)total > p[2]) final_placings += "and ";
				final_placings += numbers[p[2]] + " 3rd place" + (p[2] > 1 ? "s" : "") + (total>so_far ? ", " : "");
			}
			if (p[3]) {
				so_far += p[3];
				if (so_far == total && (int)total > p[3]) final_placings += "and ";
				final_placings += numbers[p[3]] + " 4th place" + (p[3] > 1 ? "s" : "") + (total>so_far ? ", " : "");
			}
			final_placings += "<br/>";
		}
		ret += SubSection("Final Placings", final_placings);
		return ret;
	}
};

class SimulationBranches : public Module
{
public:
	vector<UpdateEvent> branches;

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);

		auto res = mysqlcon.select(("select sim_id,unix_timestamp(created),created,size,type,description from sims left join branches using(sim_id) where base_sim_id="+ToString(simheader.sim_id)).c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		branches.clear();
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint sim_id = row[0];
			uint c = row[1];
			string screated = row[2].ToString();
			uint size = row[3];
			uint type = row[4];
			string desc = row[5].ToString();
			branches.push_back( UpdateEvent(sim_id,c,size,screated,desc,type) );
		}
	}

	virtual string InnerHtml()
	{
		string ret;
		for (auto &b : branches) {
			//if (strstr(b.description.c_str(), "Snute") && current_user.user_id!=1) continue;
			ret += "<mdiv>" + TimestampHtml(b.created) + " - <a href='" + MakeURL("sim_id=" + ToString(b.sim_id), true) + "'>Branch Sim ID #" + ToString(b.sim_id) + "</a> (" + ToStringCommas(b.samps) + " samples)";
			if (b.description.length()) ret += " - " + b.description;
			ret += "</mdiv>";
		}
		return ret;
	}
};

class MustWinMatches : public Module
{
	virtual void RunQuery() {
		upcoming.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		string ret;
		uint outted = 0;
		ShowMore showmore;
		for (auto &upm : upcomingmatches.matches) {
			bool mustwin = false;
			for (auto p : upm.pids) {
				if (upm.MustWin(p)) mustwin = true;
			}
			if (mustwin == false) continue;
			if (outted == 4) ret += showmore.Start("Upcoming Must Win Matches");
			ret += upm.Html(DEBUGARG);
			outted++;
		}
		ret += showmore.End();
		return ret;
	}
};

class MustWinTournaments : public Module
{
	virtual void RunQuery() {
		upcoming.GetData(DEBUGARG);
	}

	virtual string InnerHtml()
	{
		string ret;
		uint outted = 0;
		ShowMore showmore;
		for (auto &tt : upcomingtournaments.tournies) {
			UpcomingTournament t=tt;
			t.evgs1.clear();
			t.evgs2.clear();
			for (auto &e : tt.evgs1) {
				if (tt.MustWin(e)) {
					t.evgs1.push_back(e);
				}
			}
			if (t.evgs1.size() == 0) continue;
			//if (outted == 4) ret += showmore.Start("Must Win Tournaments");
			ret += t.Html(5000 DEBUGARGS);
			outted++;
		}
		ret += showmore.End();
		return ret;
	}
};

class FinalSeedStats : public Module
{
public:
	vector<std::pair<uint, uint>> seeds[TOP_NUM_QUALIFY];

	virtual void RunQuery() {
		//return;
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		string query = "select pid,seed,num_times from finalseedstats where sim_id=" + ToString(simheader.sim_id) + " order by num_times desc limit 5000";
		auto res = mysqlcon.select(query.c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint seed = row[1];
			seed--;
			uint num_times = row[2];
			seeds[seed].push_back(std::pair<uint, uint>(pid, num_times));
		}
	}

	virtual string InnerHtml() {
		string ret;
		string l, r;
		//return ret;
		for (uint s = 1; s <= TOP_NUM_QUALIFY; s++) {
			ShowMore showmore;
			string sec;
			/*for (uint i = 0; i < seeds[s-1].size() && i<20; i++) {
				auto seed = seeds[s - 1][i];
				if (i == 1) break;
				if (i == 3) sec += showmore.Start("Likely #" + ToString(s) + " Seeds");
				sec += "<mdiv>" + players.phtml(seed.first) + " has a "+RenderPercent(seed.second, simheader.samples)+" chance to be the #"+ToString(s)+" seed.</mdiv>";
			}
			sec += showmore.End();*/
			//ret += SubSection(""/*"Likely #" + ToString(s) + " Seeds"*/, sec);
			//if (s % 2 == 1) l += sec;// SubSection(""/*"Likely #" + ToString(s) + " Seeds"*/, sec);
			//else r += sec;// SubSection(""/*"Likely #" + ToString(s) + " Seeds"*/, sec);
			//if (s < 9) l += sec;
			//else r += sec;
			uint first_seed = s;
			uint last_seed = s;
			/*for (uint s2 = 1; s2 <= TOP_NUM_QUALIFY; s2++) {
				for (uint i = 0; i < 1; i++) {
					if (seeds[s2 - 1][i].first == seeds[s - 1][0].first) {
						first_seed = std::min<uint>(s2, first_seed);
						last_seed = std::max<uint>(s2,last_seed);
					}
				}
			}*/
			sec += "<mdiv>"+players.phtml(seeds[s - 1][0].first) + " has a " + RenderPercent(seeds[first_seed - 1][0].second, simheader.samples) + " chance to be the #" + ToString(s) + " seed";
			/*for (uint s2 = first_seed+1; s2 <= last_seed; s2++) {
				if (s2 == last_seed) sec += ", and ";
				else sec += ", ";
				sec += "a " + RenderPercent(seeds[s2 - 1][0].second, simheader.samples) + " chance to be #" + ToString(s2);
			}*/
			sec += ".</mdiv>";
			if (s < 9) l += sec;
			else r += sec;
			s = last_seed;
		}
		ret += LeftRight(l, r);
		l = "";
		r = "";

		ShowMore showall;
		ret += showall.Start("Seed Stats");
		for (uint s = 1; s <= TOP_NUM_QUALIFY; s++) {
			ShowMore showmore;
			string sec;
			for (uint i = 0; i < seeds[s - 1].size() && i < 10; i++) {
				auto seed = seeds[s - 1][i];
				//if (i == 1) break;
				if (i == 5) sec += showmore.Start("Likely #" + ToString(s) + " Seeds");
				sec += "<mdiv>" + players.phtml(seed.first) + " has a " + RenderPercent(seed.second, simheader.samples) + " chance to be the #" + ToString(s) + " seed.</mdiv>";
			}
			sec += showmore.End();
			if (s < 9) l += SubSection("Likely #" + ToString(s) + " Seeds", sec);
			else r += SubSection("Likely #" + ToString(s) + " Seeds", sec);
		}
		ret += LeftRight(l, r);
		ret += showall.End();
		return ret;
	}

	virtual string InnerHtml(uint pid) {
		string ret;
		string l, r;
		uint cnt = 1;
		for (uint s = 1; s <= TOP_NUM_QUALIFY; s++) {
			auto &seed = seeds[s - 1];
			for (uint i = 0; i < seed.size(); i++) {
				if (seed[i].first != pid) continue;
				string sec;
				sec += "<mdiv>" + players.phtml(seed[i].first) + " has a " + RenderPercent(seed[i].second, simheader.samples) + " chance to be the #" + ToString(s) + " seed.</mdiv>";
				if (cnt % 2==1) l += sec;
				else r += sec;
				cnt++;
				break;
			}
		}
		ret += LeftRight(l, r);
		return ret;
	}

	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return InnerHtml(); }
	virtual string Html(uint pid DEBUGARGSDEF) { GetData(DEBUGARG); return InnerHtml(pid); }
};
FinalSeedStats finalseedstats;

class FinalFacingStats : public Module
{
public:
	vector<array<uint, 4>> facings;//an array of 4, I'm so lazy lol - pid1, pid2, num_times, comp_num_times
	virtual void RunQuery() {
		//return;
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		string query = "select pid1,pid2,num_times, sim_id from finalfacingstats where sim_id=" + ToString(simheader.sim_id) + " or sim_id=" + ToString(simheader.comp_sim_id) + " order by sim_id=" + ToString(simheader.sim_id) + " desc, num_times desc limit 5000";
		auto res = mysqlcon.select(query.c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid1 = row[0];
			uint pid2 = row[1];
			uint num_times = row[2];
			uint sim_id = row[3];
			if (sim_id == simheader.comp_sim_id) {
				for (auto &f : facings) {
					if (f[0] == pid1 && f[1] == pid2) {
						f[3] = num_times;
						break;
					}
				}
				continue;
			}
			facings.push_back(array < uint, 4 > {{pid1, pid2, num_times, 0}});
		}
	}

	virtual string InnerHtml() {
		string ret;
		//return ret;
		ShowMore showmore;
		for (uint i = 0; i < facings.size() && i<100; i++) {
			if(i==10) ret += showmore.Start("Likely Final Matches");
			uint pid1 = facings[i][0];
			uint pid2 = facings[i][1];
			uint num_times = facings[i][2];
			uint comp_num_times = facings[i][3];
			ret += "<mdiv>" + players.phtml(pid1) + " vs " + players.phtml(pid2) + " has a " + RenderPercent(num_times, simheader.samples) + " chance for the first round of Blizzcon";
			if (comp_num_times) {
				string from = "from";
				if ((double)comp_num_times / (double)simheader.comp_samples > (double)num_times / (double)simheader.samples) from = "down from";
				if ((double)comp_num_times / (double)simheader.comp_samples < (double)num_times / (double)simheader.samples) from = "up from";
				ret += ", " + from + " " + RenderPercent(comp_num_times, simheader.comp_samples);
			}
			ret += ".</mdiv>";
		}
		ret += showmore.End();
		return ret;
	}

	virtual string InnerHtml(uint pid) {
		string ret;
		//return ret;
		ShowMore showmore;
		uint cnt = 0;
		for (uint i = 0; i < facings.size(); i++) {
			uint pid1 = facings[i][0];
			uint pid2 = facings[i][1];
			if (pid1 != pid && pid2 != pid) continue;
			if (pid2 == pid) {
				pid2 = pid1;
				pid1 = pid;
			}
			uint num_times = facings[i][2];
			uint comp_num_times = facings[i][3];
			if (cnt == 5) ret += showmore.Start("Likely Final Matches");
			ret += "<mdiv>" + players.phtml(pid1) + " vs " + players.phtml(pid2) + " has a " + RenderPercent(num_times, simheader.samples) + " chance for the first round of Blizzcon";
			if (comp_num_times) {
				string from = "from";
				if ((double)comp_num_times / (double)simheader.comp_samples > (double)num_times / (double)simheader.samples) from = "down from";
				if ((double)comp_num_times / (double)simheader.comp_samples < (double)num_times / (double)simheader.samples) from = "up from";
				ret += ", "+from+" " + RenderPercent(comp_num_times, simheader.comp_samples);
			}
			ret += ".</mdiv>";
			cnt++;
		}
		ret += showmore.End();
		return ret;
	}

	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return InnerHtml(); }
	virtual string Html(uint pid DEBUGARGSDEF) { GetData(DEBUGARG); return InnerHtml(pid); }
};
FinalFacingStats finalfacingstats;

class TournamentPlacements : public Module
{
public:
	uint tid;
	uint pid;
	vector<WebEventGroup> evgs;
	vector<uint> pids;
	vector<uint> tids;
	uint worst_place;
	uint best_place;

	TournamentPlacements(uint TID, uint PID) : Module() {
		tid = TID;
		pid = PID;
	}

	virtual void RunQuery() {
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		upcoming.GetData(DEBUGARG);

		worst_place = 1;
		best_place = 64;

		for (auto &eg : events.evgs) {
			if (eg.type != 1) continue;
			if (eg.size() != 1) continue;
			auto &e = eg.events[0];
			if (eg.pid != e.p_id) continue;
			if (pid && e.p_id != pid) continue;
			uint ttid = e.t_id;
			uint tplace = e.placing;
			if (e.placing == UPCOMING_LOSE) {
				for (auto &m : upcomingmatches.matches) {
					if (m.upm_id != ttid) continue;
					ttid = m.t_id;
					tplace = m.round;
					break;
				}
			}
			if (tplace >= 64) continue;
			if (tid && ttid != tid) continue;
			if (pid==0 && players.players[e.p_id].times_top == 0) continue;
			evgs.push_back(eg);
			evgs.back().events[0].t_id = ttid;
			evgs.back().events[0].placing = tplace;
			worst_place = std::max<uint>(worst_place, tplace);
			best_place = std::min<uint>(best_place, tplace);

			bool found = false;
			for (auto p : pids) if (p == e.p_id) found = true;
			if (found == false) pids.push_back(e.p_id);

			found = false;
			for (auto t : tids) if (t == ttid) found = true;
			if (found == false) tids.push_back(ttid);
		}
		/*std::sort(evgs.begin(), evgs.end(), [](const WebEventGroup &a, const WebEventGroup &b){
			uint times_top_a = players.players[a.pid].times_top;
			uint times_top_b = players.players[b.pid].times_top;
			if (times_top_a == times_top_b) {
				return a.pid < b.pid;
			}
			return times_top_a > times_top_b;
		});*/
		std::sort(pids.begin(), pids.end(), [](const uint a, const uint b){
			return players.players[a].rank < players.players[b].rank;
		});
		std::sort(tids.begin(), tids.end(), [](const uint a, const uint b){
			return a < b;
		});
	}

	virtual string InnerHtml() {
		string ret;
		ret += "<table class='bigtable'><thead><tr><th></th><th></th><th colspan='100'>Blizzcon Chances When Placing</th>";
		if(tid) ret += "</tr><tr><th>Player</th><th>Currently</th>";
		else if (pid) ret += "</tr><tr><th>Tournament</th><th>Currently</th>";

		vector<uint> placings;
		uint next_place = 0;
		for(uint p = worst_place; p > 0; p=next_place) {
			next_place = 0;
			for (auto &eg : evgs) {
				auto &e = eg.events[0];
				if (e.placing < p) {
					next_place = std::max<uint>(next_place, e.placing);
				}
				if (e.placing != p) continue;
			}
			placings.push_back(p);
			ret += "<th>"+IntToPlacing(p)+"</th>";
		}
		ret += "</tr></thead><tbody style='text-align:right;'>";
		//ret += "<tr><td>" + ToString(pids.size()) + "</td><td>" + ToString(tids.size()) + "</td></tr>";

		for (uint i = 0; i < pids.size()*tids.size() && i<32;i++) {
			uint p = pids[i%(uint)pids.size()];
			uint t = tids[i/(uint)pids.size()];
			if (tid) {
				ret += "<tr><td>" + players.phtml(p) + "</td>";
			}
			else if (pid) {
				ret += "<tr><td>" + tournaments.thtml(t) + "</td>";
			}
			ret += "<td>" + RenderPercent(players.players[p].times_top, simheader.samples) + "</td>";
			for (uint placingi = 0; placingi < placings.size(); placingi++) {
				uint placing = placings[placingi];
				bool found = false;
				for (auto &eg : evgs) {
					if (eg.pid != p) continue;
					auto &e = eg.events[0];
					if (e.t_id != t) continue;
					if (e.placing != placing) continue;
					ret += "<td>"+RenderPercent(eg.hits, eg.hits+eg.misses)+"</td>";
					//ret += "<td>" + ToString(placing) + "</td>";
					found = true;
					break;
				}
				if (found == false) ret += "<td></td>";
			}
			ret += "</tr>";
		}
		ret += "</tbody></table>\n";
		return ret;
	}
};

class PreviousMatches : public Module
{
public:
	uint pid;
	vector<PreviousMatch> prev_matches;
	array<uint, 10> race_records;

	PreviousMatches(uint PID) : Module()
	{
		pid = PID;
	}

	virtual void RunQuery()
	{
		for (auto &rr : race_records) rr = 0;

		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		string query = "select winner_id,loser_id,t_id,unix_timestamp(played_on),round,score_winner,score_loser from prev_matches where winner_id="+ToString(pid)+" or loser_id="+ToString(pid)+" order by played_on asc";
		auto res = mysqlcon.select(query.c_str() DEBUGARGS);
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			PreviousMatch p;
			p.winner_id = row[0];
			p.loser_id = row[1];
			p.t_id = row[2];
			p.timestamp = row[3];
			p.round = row[4];
			p.score_winner = row[5];
			p.score_loser = row[6];
			prev_matches.push_back(p);
			if (pid == p.winner_id) {
				uint opr = (uint)players.players[p.loser_id].race;
				race_records[opr * 2]++;
				race_records[8]++;
			}
			else {
				uint opr = (uint)players.players[p.winner_id].race;
				race_records[opr * 2+1]++;
				race_records[9]++;
			}
		}
	}

	string RoundOf(uint round)
	{
		string sround = "round of " + ToString(round);
		if (round == 8) sround = "quarterfinals";
		else if (round == 4) sround = "semifinals";
		else if (round == 2) sround = "finals";
		return sround;
	}

	virtual string InnerHtml()
	{
		string ret;
		for (auto p : prev_matches) {
			if (p.loser_id == pid) ret += "<mdiv>" + DateHtml(p.timestamp) + ": " + players.pshorthtml(p.loser_id) + " lost to " + players.pshorthtml(p.winner_id)
				+ " in " + tournaments.tournies[p.t_id].name + " " + RoundOf(p.round)
				/*+ " "+ToString(p.score_winner)+" to "+ToString(p.score_loser)*/ + "</mdiv>";
			else ret += "<mdiv>"+DateHtml(p.timestamp)+": " + players.pshorthtml(p.winner_id) + " beat " + players.pshorthtml(p.loser_id)
				+ " in " + tournaments.tournies[p.t_id].name + " " + RoundOf(p.round)
				/*+ " "+ToString(p.score_winner)+" to "+ToString(p.score_loser)*/+"</mdiv>";
		}
		ret += "<mdiv>Total: "+ToString(race_records[8])+"-"+ ToString(race_records[9]) +", vsT: " + ToString(race_records[(uint)T * 2]) + "-" + ToString(race_records[(uint)T * 2 + 1]) + ", vsZ: " + ToString(race_records[(uint)Z * 2]) + "-" + ToString(race_records[(uint)Z * 2 + 1]) + ", vsP: " + ToString(race_records[(uint)P * 2]) + "-" + ToString(race_records[(uint)P * 2 + 1]) + /*", vsR: " + ToString(race_records[(uint)R * 2]) + "-" + ToString(race_records[(uint)R * 2 + 1]) +*/ "</mdiv>";
		return ret;
	}
};

class Retrospective : public Module
{
public:
	uint pid;
	PreviousMatches mod_prev_matches;
	vector<array<uint, 8>> history;
	vector<string> notes;

	Retrospective(uint PID) : Module(), mod_prev_matches(PID)
	{
		pid = PID;
	}

	string UpdateNotes(string note, uint &ns, vector<UpdateEvent> &update_events, uint created, bool all)
	{
		all = true;
		if (note.length()) note += ", ";
		uint time_span = 3600 * 12;
		if (all) time_span = 86400 * 3;
		while (ns < update_events.size() && update_events[ns].created <= created) {
			if ( (update_events[ns].created+time_span > created) && update_events[ns].description.length()>0) note += update_events[ns].description + ", ";
			ns++;
		}
		if (note.length()) note = note.substr(0, note.length() - 2);
		return note;
	}

	void PushHistory(string extra_note, array < uint, 8 > &vals, array < uint, 8 > &last_vals, uint &ns, vector<UpdateEvent> &update_events, bool all_notes, bool check_append, bool allow_append)
	{
		static uint orig_created = 0;
		string note;
		bool append = allow_append;//maybe add more logic later
		if (history.size() <= 1) append = false;
		if (history.size() && orig_created + 3600*18 > vals[7] && append) {
			note = extra_note;
			note = UpdateNotes(note, ns, update_events, vals[7], all_notes);
			if (note.length() && notes.back().length()) notes.back() += ", " + note;
			else if (note.length()) notes.back() += note;
			history.back() = vals;
			return;
		}
		if (check_append) return;
		note = UpdateNotes(note, ns, update_events, last_vals[7], all_notes);
		if (last_vals[6] > 0) {
			history.push_back(last_vals);
			notes.push_back(note);
			note = "";
		}
		if (note.length() && extra_note.length()) note += ", " + extra_note;
		else if (extra_note.length()) note += extra_note;
		note = UpdateNotes(note, ns, update_events, vals[7], all_notes);
		history.push_back(vals);
		notes.push_back(note);
		orig_created = vals[7];
		//vals[6] = 0;//don't reinsert this
	}

	void AmmendHistory(string extra_note, array < uint, 8 > &vals, array < uint, 8 > &last_vals, uint &ns, vector<UpdateEvent> &update_events, bool all_notes)
	{
	}

	virtual void RunQuery()
	{
		simheader.GetData(DEBUGARG);
		players.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		mod_prev_matches.GetData(DEBUGARG);
		auto &prev_matches = mod_prev_matches.prev_matches;
		auto &update_events = simheader.update_events;
		string query = playerhistorygraphsdata.GetQueryString(simheader.comp_created, simheader.created, ToString(pid));

		auto * res = mysqlcon.select(query.c_str() DEBUGARGS);

		uint pms = 0;//prev_match_slot of course
		uint ns = 0;//last note slot
		string last_month = "none";
		array<uint, 8> start_month = { { 0 } };
		const uint min_match_len = 60 * 30;//give 30 minutes for a match

		array < uint, 8 > last_vals = { { 0, 0, 0, 0, 0, 0 } };
		auto num_rows = mysql_num_rows(res);
		for (decltype(num_rows) r = 0; r < num_rows; r++) {
			auto row = mysqlcon.fetch_row(res);
			uint pid = row[0];
			uint times_top = row[1];
			uint sim_id = row[2];
			if (sim_id < simheader.comp_sim_id) continue;
			
			uint size = row[3];
			uint created = row[4];

			//while (sim_id == simheader.comp_sim_id && ns < update_events.size() && update_events[ns].created < created) ns++;

			uint minpoints = row[5];
			uint medianpoints = row[6];
			//uint meanpoints = row[7];
			uint afk_times_top = row[8];
			uint afk_samps = row[9];
			if (afk_samps == 0) {
				afk_times_top = 0;
				afk_samps = 100;
			}

			array < uint, 8 > vals = { { times_top, size, minpoints, medianpoints, afk_times_top, afk_samps, sim_id, created } };

			time_t rawtimestamp = (time_t)created;
			char buffer[128];
			tm * timeinfo;
			timeinfo = gmtime(&rawtimestamp);
			strftime(buffer, 128, "%B %Y", timeinfo);
			string month = buffer;

			double prob = (double)times_top / (double)size *100.0;
			double afk_prob = (double)afk_times_top / (double)afk_samps *100.0;

			double prev_prob = prob;
			double prev_afk_prob = afk_prob;
			if (last_vals[1] > 0) {
				prev_prob = (double)last_vals[0] / (double)last_vals[1] * 100.0;
				if (last_vals[5] > 0) {
					prev_afk_prob = (double)last_vals[4] / (double)last_vals[5] * 100.0;
				}
			}

			/*if (history.size() && history.back()[7]+86400 < created) {
				AmmendHistory("", vals, last_vals, ns, update_events, true);
				start_month = vals;
				last_month = month;
				} else */if (pms<prev_matches.size() && created > prev_matches[pms].timestamp + min_match_len) {
				string extra_note;
				while (pms<prev_matches.size() && created > prev_matches[pms].timestamp + min_match_len) {
					if (extra_note.length()) extra_note += ", ";
					if (prev_matches[pms].loser_id == pid) extra_note += players.players[prev_matches[pms].loser_id].name.ToString() + " lost to " + players.players[prev_matches[pms].winner_id].name.ToString() + " in " + tournaments.tournies[prev_matches[pms].t_id].name + " " + mod_prev_matches.RoundOf(prev_matches[pms].round);
					else extra_note += players.players[prev_matches[pms].winner_id].name.ToString() + " beat " + players.players[prev_matches[pms].loser_id].name.ToString() + " in " + tournaments.tournies[prev_matches[pms].t_id].name + " " + mod_prev_matches.RoundOf(prev_matches[pms].round);
					pms++;
				}
				PushHistory(extra_note, vals, last_vals, ns, update_events, true, false, false);
				start_month = vals;
				last_month = month;
			}
			else if (minpoints != last_vals[2]) {
				PushHistory(""/*"min points"*/, vals, last_vals, ns, update_events, false, false, false);
				start_month = vals;
				last_month = month;
			}
			else if (abs(prob - prev_prob) > (prev_prob + 1.0)*0.2) {
				PushHistory(""/*"chances"*/, vals, last_vals, ns, update_events, false, false, true);
				start_month = vals;
				last_month = month;
			}
			else if (abs(afk_prob - prev_afk_prob) > (prev_afk_prob + 5.0)*0.25) {
				PushHistory(""/*"afk chances"*/, vals, last_vals, ns, update_events, false, false, true);
				start_month = vals;
				last_month = month;
			}
			else if (abs((int)medianpoints - (int)last_vals[3]) > 100) {
				PushHistory(""/*"median points"*/, vals, last_vals, ns, update_events, false, false, true);
				start_month = vals;
				last_month = month;
			}
			else if (last_vals[1] == 0) {
				PushHistory(""/*"start"*/, vals, last_vals, ns, update_events, false, false, true);
				start_month = vals;
				last_month = month;
			}
			else if (month != last_month) {
				PushHistory(""/*"new month"*/, vals, last_vals, ns, update_events, true, false, false);
				start_month = vals;
				last_month = month;
			}
			else {
				PushHistory(""/*"check append"*/, vals, last_vals, ns, update_events, true, true, true);
				start_month = vals;
				last_month = month;
			}
			last_vals = vals;
		}
	}

	virtual string InnerHtml()
	{
		string ret;
		string last_month = "none";
		string month_section;
		array<uint, 8> start_month = { { 0 } };

		for (uint i = 0; i < history.size();i+=2) {
			auto &h = history[i];
			//{ times_top, size, minpoints, medianpoints, afk_times_top, afk_samps, sim_id, created }
			uint times_top = h[0];
			uint size = h[1];
			uint minpoints = h[2];
			uint medianpoints = h[3];
			uint afk_times_top = h[4];
			uint afk_samps = h[5];
			uint sim_id = h[6];
			uint created = h[7];

			time_t rawtimestamp = (time_t)created;
			char buffer[128];
			tm * timeinfo;
			timeinfo = gmtime(&rawtimestamp);
			strftime(buffer, 128, "%B %Y", timeinfo);
			string month = buffer;

			if (i == 0) {
				last_month = month;
				//month_section += "<mdiv>Start "+TimestampHtml(created)+" - "+RenderPercent(times_top, size)+" - "+notes[i]+"</mdiv>";
				month_section += "<mdiv>" + DateHtml(created) + " - " + RenderPercent(times_top, size) + " - " + notes[i] + "</mdiv>";
				start_month = h;
				continue;
			}

			auto &prev_h = history[i - 1];
			uint prev_times_top = prev_h[0];
			uint prev_size = prev_h[1];
			uint prev_minpoints = prev_h[2];
			uint prev_medianpoints = prev_h[3];
			uint prev_afk_times_top = prev_h[4];
			uint prev_afk_samps = prev_h[5];
			uint prev_sim_id = prev_h[6];
			uint prev_created = prev_h[7];

			double afk_prob = (double)afk_times_top / (double)afk_samps * 100.0;
			double prev_afk_prob = (double)prev_afk_times_top / (double)prev_afk_samps * 100.0;

			if (month != last_month) {
				string title = last_month + " - From " + RenderPercent(start_month[0], start_month[1]) + " to " + RenderPercent(prev_times_top, prev_size) +", "+ToStringCommas(start_month[2])+" WCS Points to "+ToStringCommas(prev_minpoints)+", "+ToStringCommas(start_month[3])+" Median Points to "+ToStringCommas(prev_medianpoints);
				if (globalcontext.r == Renderer::bbcode) title = "[big]" + title + "[/big]";
				if (month_section.length()) ret += "<br/>" + SubSection(title, month_section);
				last_month = month;
				month_section = "";
				start_month = h;
			}

			string date_range;
			//if (prev_created + 86400 < created) date_range = TimestampHtml(prev_created) + " to ";
			//if (prev_created + 86400 < created) date_range = DateHtml(prev_created) + " to ";
			if (DateHtml(prev_created) != DateHtml(created)) date_range = DateHtml(prev_created) + " to ";
			//date_range += TimestampHtml(created);
			date_range += DateHtml(created);

			month_section += "<mdiv><a href=\"" +MakeURL("sim_id="+ToString(sim_id)+"&comp_sim_id="+ToString(prev_sim_id), true)+ "\">"+date_range+"</a> - "+notes[i];
			if (notes[i].length()) month_section += " - ";
			month_section += "going from " + RenderPercent(prev_times_top, prev_size) + " to " + RenderPercent(times_top, size);
			if (abs(afk_prob - prev_afk_prob) > (prev_afk_prob + 5.0)*0.25) month_section += ", AFK Chances from " + RenderPercent(prev_afk_times_top, prev_afk_samps) + " to " + RenderPercent(afk_times_top, afk_samps);
			if(prev_minpoints!=minpoints) month_section += ", min points from " + ToStringCommas(prev_minpoints) + " to " + ToStringCommas(minpoints);
			else month_section += ", min points " + ToStringCommas(minpoints);
			if (prev_medianpoints != medianpoints) month_section += ", median points from " + ToStringCommas(prev_medianpoints) + " to " + ToStringCommas(medianpoints);
			else month_section += ", median points " + ToStringCommas(medianpoints);
			month_section += "</mdiv>";
		}
		if (month_section.length()) {
			auto &prev_h = history.back();
			uint prev_times_top = prev_h[0];
			uint prev_size = prev_h[1];
			uint prev_minpoints = prev_h[2];
			uint prev_medianpoints = prev_h[3];
			//uint prev_afk_times_top = prev_h[4];
			//uint prev_afk_samps = prev_h[5];
			//uint prev_sim_id = prev_h[6];
			//uint prev_created = prev_h[7];
			//string title = last_month + " - From " + RenderPercent(start_month[0], start_month[1]) + " to " + RenderPercent(history.back()[0], history.back()[1]);
			string title = last_month + " - From " + RenderPercent(start_month[0], start_month[1]) + " to " + RenderPercent(prev_times_top, prev_size) + ", " + ToStringCommas(start_month[2]) + " WCS Points to " + ToStringCommas(prev_minpoints) + ", " + ToStringCommas(start_month[3]) + " Median Points to " + ToStringCommas(prev_medianpoints);
			if (globalcontext.r == Renderer::bbcode) title = "[big]" + title + "[/big]";
			ret += "<br/>" + SubSection(title, month_section);
		}
		return ret;
	}
};
