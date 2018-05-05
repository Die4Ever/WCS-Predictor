
string DefaultSimHeaderSection(bool hidden=true)
{
	string ret;
	simheader.GetData(DEBUGARG);
	string age_string = simheader.AgeString();
	string title = "Info - " + age_string + " - " + ToStringCommas(simheader.samples) + " samples";
	string classes = "sim-info-section";
	if (simheader.type != 1 || simheader.branch_desc.length()>0 || vars["sim_id"].length()>0) {
		classes += " section-red";
		ret += "<style>body {background:#caa !important;}</style>";
	}
	if (simheader.branch_desc.length()) {
		title = simheader.branch_desc +" - Branch "+ title;
	}
	else if (simheader.type != 1) {
		title = "Branch " + title;
	}
	else {
		title = "Simulation " + title;
	}
	if (simheader.samples == 0) hidden = false;
	ret += Section(title, simheader.Html(DEBUGARG), classes, hidden);
	return ret;
}

class Page : public Module {
public:
	string bodyclass;
	string title;

	Page()
	{
		bodyclass = "default-page";
		title = "WCS Predictor 2015";
	}

	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return HtmlPrefix() + InnerHtml() + HtmlPostfix(); }

	virtual string HtmlPrefix() {
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + Header(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		//need a module for HeaderBar/NavBar? also needs to load alerts and stuff so yea it needs to be a global module instead of a function
		pre += "\n\n" + headerbar.Html(DEBUGARG);
		pre += "\n\n";
		//simheader.GetData(DEBUGARG);
		//string age_string=simheader.AgeString();
		//pre += Section("Simulation Info - "+age_string+" - "+ToStringCommas(simheader.samples)+" samples", simheader.Html(DEBUGARG), "sim-info-section", true);
		pre += DefaultSimHeaderSection();
		pre += "\n\n";
		pre += current_user.Html(DEBUGARG) + "\n\n";
		pre += wwglobals.Html(DEBUGARG) + "\n\n";
		return pre;
	}
	virtual string HtmlPostfix() {
		string post = "\n\n" + Footer(false DEBUGARGS);
		return post;
	}

	virtual string InnerHtml()
	{
		return Section("Unknown Page", "This page cannot be found. Go back <a href='"+MakeURL("")+"'>Home</a>", "unknown-page-section");
	}
};

//HomePage, Login, PlayerPage, PlayerList, MustWins, FringePlayers...
//PlayerSetPage subclasses - TournamentPage, TeamPage, RacePage, CountryPage, FavoritePlayers
//	-do these need to be subclasses since it'll pretty much just be calling modules?

class HomePage : public Page
{
public:
	HomePage()
	{
		bodyclass = "home-page";
	}

	virtual string InnerHtml()
	{
		simheader.GetData(DEBUGARG);
		if (simheader.sim_id == 0) {
			return Section("Error", "Invalid Simulation ID.", "error-section");
		}
		if (simheader.samples == 0) {
			return Section("Error", "This simulation doesn't have any samples yet.", "error-section");
		}
		string ret;
		uint days=(((uint)time(0)-simheader.comp_created)+(86400/2))/86400;
		TopPlayers topplayers(25);
		ret += Section("Top 25 Players (<a href=\"" + MakeURL("page=players")+"\">Click Here for Full Player List</a>)", topplayers.Html(DEBUGARG), "top-players-section flush-section");
		ret += Section("Upcoming", upcoming.Html(DEBUGARG), "upcoming-section");
		//ret += biggest_winners_losers.Html(DEBUGARG);
		ret += Section("Biggest Winners In The Past "+ToString(days)+" Days", biggest_winners_losers.WinnersHtml(), "biggest-winners-section");
		ret += Section("Biggest Losers In The Past "+ToString(days)+" Days", biggest_winners_losers.LosersHtml(), "biggest-losers-section");
		ret += Section("Estimated Representatives", summedpercentages.Html(DEBUGARG), "summed-percentages-section");
		ret += Section("Headbands", headbands.Html(DEBUGARG), "headbands-section");
		ret += Section("Point Cutoffs", wcspcutoffs.Html(DEBUGARG), "point-cutoffs-section");
		ret += Section("Foreigner Hope", foreigner_hopes.Html(DEBUGARG)+"<rhr style='visibility:hidden;'></rhr>"+summedpercentages.CountriesHtml(DEBUGARG), "foreigner-hope-section");
		//ret += Section("Final Seed Stats", LeftRight(finalseedstats.Html(DEBUGARG),finalfacingstats.Html(DEBUGARG)), "final-seed-section");
		ret += Section("Final Seed Stats", finalseedstats.Html(DEBUGARG), "final-seed-section");
		ret += Section("Final Match Stats", finalfacingstats.Html(DEBUGARG), "final-match-section");
		//ret += Section("Tournament Stuff", "I'm not sure yet how the tournament stuff should be organized, previously it was 3 columns of winning chances, but I would rather have 1 tournament per row instead of having 3 tournaments per row, maybe I should take this section out and just show the tournaments in the upcoming section?", "tournaments-section");
		ret += playerhistorygraphsdata.Html(DEBUGARG);
		/*UpcomingHighlights upcoming;
		SummedRaces summedraces;
		SummedTeams summedteams;
		string highlightshtml;
		highlightshtml+=upcoming.Html();//upcominghighlights should make its own subsection?
		ret += Section("Highlights", highlightshtml, "highlights-section");*/
		return ret;
	}
};

class PlayerListPage : public Page
{
public:
	PlayerListPage()
	{
		bodyclass = "player-list-page";
	}
	
	virtual string InnerHtml()
	{
		string ret;
		TopPlayers topplayers(25000);
		ret += Section("Players", topplayers.Html(DEBUGARG), "players-section flush-section");
		return ret;
	}
};

class TournamentsPage : public Page
{
public:
	TournamentsPage()
	{
		bodyclass = "tournaments-page";
	}

	virtual string InnerHtml()
	{
		string ret;
		upcomingtournaments.GetData(DEBUGARG);
		for(auto &t : upcomingtournaments.tournies) {
			if(t.evgs1.size()==0) continue;
			ret += Section(tournaments.thtml(t.t_id), t.Html(100 DEBUGARGS), "tournament-section-"+ToString(t.t_id)+" tournament-section");
		}
		return ret;
	}
};

class SimulationsPage : public Page {
public:
	virtual string HtmlPrefix() {
		//vars["comp_days"]="60";
		vars["comp_sim_id"] = "1079";
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + Header(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		//need a module for HeaderBar/NavBar? also needs to load alerts and stuff so yea it needs to be a global module instead of a function
		pre += "\n\n" + headerbar.Html(DEBUGARG);
		pre += "\n\n";
		//simheader.GetData(DEBUGARG);
		//string age_string=simheader.AgeString();
		//pre += Section("Simulation Info - "+age_string+" - "+ToStringCommas(simheader.samples)+" samples", simheader.Html(DEBUGARG), "sim-info-section");
		pre += DefaultSimHeaderSection(false);
		pre += "\n\n";
		pre += current_user.Html(DEBUGARG) + "\n\n";
		return pre;
	}
	
	virtual string InnerHtml()
	{
		string ret;
		return ret;
	}
};

class CountriesPage : public Page {
public:
	virtual string InnerHtml()
	{
		string ret;
		ret += Section("Countries", "Countries page coming soon.", "countries-page-section");
		return ret;
	}
};

class MustWinPlayersPage : public Page {
public:
	virtual string InnerHtml()
	{
		string ret;
		//ret += Section("Must Win Players", "Must Win Players page coming soon.", "must-win-players-section");

		MustWinMatches matches;
		MustWinTournaments tournies;

		ret += Section("Must Win Matches", matches.Html(DEBUGARG), "must-win-matches-section upcoming-section");
		ret += Section("Must Win Tournaments", tournies.Html(DEBUGARG), "must-win-tournaments-section");
		return ret;
	}
};

class PlayerSetPage : public Page
{
public:
	vector<uint> pids;
	PlayerSetPage()
	{
		bodyclass = "playerset-page";
	}

	virtual void RunQuery()
	{//why not use this?
		players.GetData(DEBUGARG);
		vector<std::pair<uint, uint> > pids_matches(MAX_PLAYER_ID);
		for (auto &p : pids_matches) p.first = p.second = 0;

		uint searches = 0;
		if (vars["c"]) {
			searches++;
			vector<string> countries;
			SplitBy(ToLower(vars["c"]).c_str(), '+', countries);
			for (auto &s : countries) {
				for (auto &p : players.top_players) {
					if (p.sCountry() != s && s != "nonkorean") continue;
					if (s == "nonkorean" && p.sCountry() == "kr") continue;
					pids_matches[p.player_id].first = p.player_id;
					pids_matches[p.player_id].second++;
				}
			}
		}
		if (vars["r"]) {
			searches++;
			vector<string> races;
			SplitBy(ToLower(vars["r"]).c_str(), '+', races);
			for (auto &s : races) {
				char r = R;
				if (s == "p") r = P;
				if (s == "t") r = T;
				if (s == "z") r = Z;

				for (auto &p : players.top_players) {
					if (p.race != r) continue;
					pids_matches[p.player_id].first = p.player_id;
					pids_matches[p.player_id].second++;
				}
			}
		}
		if (vars["teams"]) {
			searches++;
			vector<string> teams;
			SplitBy(vars["teams"].c_str(), '+', teams);
			for (auto &s : teams) {
				uint tid = ToUInt(s.c_str());
				for (auto &p : players.top_players) {
					if (p.team_id != (int)tid) continue;
					pids_matches[p.player_id].first = p.player_id;
					pids_matches[p.player_id].second++;
				}
			}
		}
		if (vars["pids"]) {
			searches++;
			//searches++;
			vector<string> spids;
			SplitBy(vars["pids"].c_str(), '+', spids);
			for (auto &s : spids) {
				uint pid = ToUInt(s.c_str());
				for (auto &p : players.top_players) {
					if (p.player_id != pid) continue;
					pids_matches[p.player_id].first = p.player_id;
					pids_matches[p.player_id].second++;
				}
			}
		}
		if (vars["or"] == "1") {
			searches = 1;
		}
		for (auto &p : pids_matches) {
			if (p.second<searches) continue;
			if (p.first == 0) continue;
			pids.push_back(p.first);
		}
	}

	virtual string InnerHtml()
	{
		string ret;
		ret += PlayerSetHtml();
		ret += PlayerSetGraphs();
		ret += Section("Events", PlayerSetEvents(pids).Html(DEBUGARG), "player-set-events-section flush-section");
		return ret;
	}

	string PlayerSetGraphs()
	{
		players.GetData(DEBUGARG);
		vector<uint> top_pids;
		string spids;
		for (uint i = 0; i < players.top_players.size() && top_pids.size() < 10; i++) {
			for (auto p : pids) {
				if (p == players.top_players[i].player_id) {
					top_pids.push_back(p);
					break;
				}
			}
		}
		for (auto p : top_pids) {
			spids += ToString(p) + ",";
			playerhistorygraphsdata.pids.push_back(p);
		}

		string ret;
		ret += Section("Probability Histories", "<div class='graph player-history-graph' data-pids='" + spids + "'></div>", "players-chances-history-section");
		ret += Section("WCS Points Histories", "<div class='graph wcspoints-history-graph' data-pids='" + spids + "'></div>", "players-wcspoints-history-section");

		playerhistorygraphsdata.mode = 2;
		//playerhistorygraphsdata.pids.push_back(pid);
		playerhistorygraphsdata.start = simheader.created - 86400 * HISTORY_DAYS;
		playerhistorygraphsdata.end = simheader.created;
		ret += playerhistorygraphsdata.Html(DEBUGARG);
		return ret;
	}

	string PlayerSetHtml()
	{
		players.GetData(DEBUGARG);
		
		PlayerSetTable pst(pids);
		PlayerSetUpcoming psups(pids);
		string ret;
		ret += Section("Player Set", pst.Html(DEBUGARG), "player-set-list flush-section");
		ret += Section("Upcoming For Player Set", psups.Html(DEBUGARG), "upcoming-section");
		return ret;
	}
};

class TournamentPage : public Page
{
public:
	uint tid;
	TournamentPage()
	{
		tid=vars["tid"];
		bodyclass = "tournament-page tournament-page-"+ToString(tid);
	}

	virtual string InnerHtml()
	{
		tournaments.GetData(DEBUGARG);
		upcoming.GetData(DEBUGARG);
		TournamentUpcoming tu(tid);
		tu.GetData(DEBUGARG);
		TournamentPlacements placements(tid, 0);
		placements.GetData(DEBUGARG);
		vector<uint> pids;
		for(auto &t : tu.upts.tournies) {
			for(auto &e : t.evgs1) {
				pids.push_back(e.pid);
			}
		}
		PlayerSetTable pst(pids);

		string ret;
		ret+=Section("Upcoming for "+tournaments.thtml(tid), tu.upms.Html(DEBUGARG), "tournament-upcoming upcoming-section");
		for(auto & t : tu.upts.tournies) {
			ret+=Section("Winning Chances for "+tournaments.thtml(tid), t.Html(100 DEBUGARGS), "tournament-winning-chances");
		}
		ret += Section("Placements Table for " + tournaments.thtml(tid), placements.Html(DEBUGARG), "tournament-placements-section flush-section");
		ret+=Section("Players for "+tournaments.thtml(tid), pst.Html(DEBUGARG), "tournament-players flush-section");

		PlayerSetPage psetp;
		psetp.pids = pids;
		ret += psetp.PlayerSetGraphs();
		ret += Section("Events", TournamentEvents(tid).Html(DEBUGARG), "tournament-events-section flush-section");

		if (vars["groupofdeath"]) {
			GroupOfDeath god(tid, tu);
			ret += Section("Group of Death", god.Html(DEBUGARG), "group-of-death-section");
		}
		return ret;
	}
};

class PlayerPage : public Page
{
public:
	PlayerPage()
	{
		bodyclass = "player-page";
	}

	virtual string InnerHtml()
	{
		//DEBUGOUT(ERR, simheader.sim_id, vars["sim_id"].ToString());
		simheader.GetData(DEBUGARG);
		//DEBUGOUT(ERR, simheader.sim_id, vars["sim_id"].ToString());
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		upcoming.GetData(DEBUGARG);

		uint pid=vars["pid"];
		auto &p=players.players[pid];

		TournamentPlacements placements(0, pid);
		placements.GetData(DEBUGARG);
		PreviousMatches previous_matches(pid);

		string ret;
		string pinfo;
		uint rank=0;
		for(uint r=0;r<players.top_players.size();r++) {
			if(players.top_players[r].player_id==pid) {
				rank=r+1;
				break;
			}
		}
		double prob=(double)p.times_top/(double)simheader.samples*100.0;
		double oldprob=(double)p.comp_times_top/(double)simheader.comp_samples*100;
		string psubinfo;
		psubinfo+="Rank #"+ToString(rank);
		if(p.team_id) {
			psubinfo+="<br/>On team "+players.teamhtml(p.team_id);
		}
		if(p.league) {
			psubinfo+="<br/>Currently in "+tournaments.thtml(p.league);
		}

		string wcspointsstats;
		wcspointsstats+="Minimum: "+ToString(p.minpoints)+"<br/>";
		wcspointsstats+="Chances to end up with "+ToString(p.minpoints)+" points: "+RenderPercent(p.afk_samps, simheader.samples, 100.0)+"<br/>";
		wcspointsstats+="AFK Chances: "+RenderPercent(p.afk_times_top, p.afk_samps, players.cutoff)+"<br/>";
		wcspointsstats+="Maximum: "+ToString(p.maxpoints)+"<br/>";
		wcspointsstats+="Mean: "+ToString(p.meanpoints)+"<br/>";
		wcspointsstats+="Median: "+ToString(p.medianpoints)+"<br/>";
		wcspointsstats+="Mode: "+ToString(p.modepoints)+"<br/>";
		if(p.minpoints_qualify==987654321)
			wcspointsstats+="Minimum while qualifying: NA<br/>";
		else
			wcspointsstats+="Minimum while qualifying: "+ToString(p.minpoints_qualify)+"<br/>";
		if(p.maxpoints_notqualify==0)
			wcspointsstats+="Maximum while not qualifying: N/A<br/>";
		else
			wcspointsstats+="Maximum while not qualifying: "+ToString(p.maxpoints_notqualify)+"<br/>";

		wcspointsstats += "WCS Wars Value: " + ToString(PlayerValue(pid)) + "<br/>";

		string aligulacratings;
		aligulacratings+="vs <a href='"+MakeURL("r=R")+"'><img src='/b.gif' class='r-ico'></img></a>: "+ToString(p.ratings[(uint)R])+"<br/>";
		aligulacratings+="vs <a href='"+MakeURL("r=P")+"'><img src='/b.gif' class='p-ico'></img></a>: "+ToString(p.ratings[(uint)P])+"<br/>";
		aligulacratings+="vs <a href='"+MakeURL("r=T")+"'><img src='/b.gif' class='t-ico'></img></a>: "+ToString(p.ratings[(uint)T])+"<br/>";
		aligulacratings+="vs <a href='"+MakeURL("r=Z")+"'><img src='/b.gif' class='z-ico'></img></a>: "+ToString(p.ratings[(uint)Z])+"<br/>";

		pinfo+=LeftRight(
			SubSection(RenderPercent(p.times_top, simheader.samples, players.cutoff)+ChangeArrow(prob,oldprob)+" chances to qualify for Blizzcon<br/>("+ToStringCommas(p.times_top)+" / "+ToStringCommas(simheader.samples)+")", psubinfo)+SubSection("Aligulac Ratings", aligulacratings),
			SubSection("WCS Points Stats", wcspointsstats));
		/*pinfo+=SubSection(RenderPercent(p.times_top, simheader.samples, players.cutoff)+ChangeArrow(prob,oldprob)+" chances to qualify for Blizzcon<br/>("+ToStringCommas(p.times_top)+" / "+ToStringCommas(simheader.samples)+")", psubinfo);
		pinfo+=LeftRight( SubSection("WCS Points Stats", wcspointsstats),SubSection("Aligulac Ratings", aligulacratings));*/
		ret+=Section(players.phtml(pid), pinfo, "player-info-section");

		ret+=Section("Upcoming for "+players.pshorthtml(pid), upcoming.HtmlPlayer(pid DEBUGARGS), "player-upcoming-section");
		ret += Section("Final Seed Stats for " + players.pshorthtml(pid), finalseedstats.Html(pid DEBUGARGS), "player-final-seed-stats-section");
		ret += Section("Final Match Stats for " + players.pshorthtml(pid), finalfacingstats.Html(pid DEBUGARGS), "player-final-match-stats-section");
		ret+=Section("Probability History for "+players.pshorthtml(pid), "<div class='graph player-history-graph' data-pids='"+ToString(pid)+"'></div>", "player-chances-history-section");
		ret+=Section("WCS Points History for "+players.pshorthtml(pid), "<div class='graph wcspoints-history-graph' data-pids='"+ToString(pid)+"'></div>", "player-wcspoints-history-section");
		ret += Section("Placements Table for " + players.pshorthtml(pid), placements.Html(DEBUGARG), "player-tournament-placings-section flush-section");
		ret+=Section("Events that will help or hurt "+players.pshorthtml(pid)+"'s chances of qualifying for Blizzcon", PlayerEvents(pid).Html(DEBUGARG), "player-events-section flush-section");
		ret += Section("Previous Matches for "+players.pshorthtml(pid), previous_matches.Html(DEBUGARG), "player-previous-matches-section");

		if (vars["retro"] == "1") {
			Retrospective retro(pid);
			ret += Section("Retrospective for "+players.pshorthtml(pid), retro.Html(DEBUGARG), "player-retro-section");
		}

		playerhistorygraphsdata.mode=2;
		playerhistorygraphsdata.pids.push_back(pid);
		playerhistorygraphsdata.start=simheader.created-86400*HISTORY_DAYS;
		playerhistorygraphsdata.end=simheader.created;
		ret += playerhistorygraphsdata.Html(DEBUGARG);
		return ret;
	}
};

class HelpPage : public Page
{
public:
	HelpPage()
	{
		bodyclass = "help-page";
	}
	
	/*virtual string HtmlPrefix() {
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + Header(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		pre += "\n\n" + headerbar.Html(DEBUGARG);
		pre += "\n\n" + current_user.Html(DEBUGARG) + "\n\n";
		return pre;
	}*/
	
	virtual string InnerHtml()
	{
		string ret;
		string faq;
		faq += SubSection("What is WCS Predictor?","WCS Predictor is a website that gives probabilites for players qualifying for Blizzcon/WCS Global Finals, along with many other statistics.");
		faq += SubSection("How does it work?","It works by running millions of simulations of all the tournaments using the Monte Carlo method (<a href='http://en.wikipedia.org/wiki/Monte_Carlo_method'>wikipedia link</a>), it makes use of Aligulac ratings to calculate expected winrates for every match that happens in each simulation. You can see details on the current simulation and the simulation being compared to by expanding the Simulaton Info section at the top of the page.");
		faq += SubSection("Where can I ask questions and discuss WCS Predictor and WCS Wars?", "Go to the discussion thread on Team Liquid! <a href='http://www.teamliquid.net/forum/starcraft-2/471444-wcs-predictor-2015'>http://www.teamliquid.net/forum/starcraft-2/471444-wcs-predictor-2015</a>. Also you can follow me on Twitter <a href='https://twitter.com/Die4EverDM'>@Die4EverDM</a>.");

		faq += SubSection("How often is this updated?","Usually every day that there are WCS matches, sometimes more than once a day like during a Dreamhack or IEM. You can see the graphs to see how many points they have, that shows you how often it's updated, or just check the thread on Team Liquid.");
		faq += SubSection("Why does &lt;my favorite foreigner&gt; have lower chances than &lt;faceless korean&gt; even though he has more points?","Usually this is due to Aligulac ratings, tournaments that they are confirmed to be attending, or even what region of WCS they play in.");

		faq += SubSection("Why did X Player's chances change even though they didn't play anything?", "Aligulac ratings, and other players doing stuff. Consider that there are 1600% chances total in the pool, so if a player loses 1% then that 1% is spread out amongst the other players.");
		faq += SubSection("Does 0% really mean no chance? Does 100% really mean confirmed?", "Pretty much, just don't get too comfortable with a 90+% chance (sOs...). Looking at the AFK Chances can also help with this, see below.");
		faq += SubSection("What are the AFK Chances?", "WCS Predictor counts how likely they are to end up with their minimum number of WCS Points, and also how many times they qualify for Blizzcon with that many points. Basically it's what their chances would be if they don't gain anymore WCS Points.");
		faq += SubSection("What are the Headbands?", "The headbands are based on the idea of the headbands in Afro Samurai (<a href='http://en.wikipedia.org/wiki/Afro_samurai#Plot'>wikipedia link to Afro Samurai plot</a>). There will be 4 headbands this year instead of just 2, so hopefully we'll see the #1 headband move around more. The headbands were seeded based on the results from Blizzcon, so Life started with the #1, MMA with #2, TaeJa with #3, Classic with the #4. The #1 headband can only lose their headband by losing to a player who holds one of the other headbands. The other headbands are lost when they lose to anyone. If 2 headband holders face each other then the winner takes the better headband and the loser takes the worse one. There is no functional difference between the #2/#3/#4. These are only tracked for tournaments that give WCS Points, and no qualifiers.");

		faq += SubSection("How can you predict DreamHack Antarctica when the player list hasn't even been announced yet?","The player selection is randomized so that in some of the samples it might consider Hyun as attending, and in some other samples it might not. Tournament attendence selection is weighted towards players with Aligulac ratings over 1300.");
		faq += SubSection("What are the Placeholder tournaments?","These are tournaments that hold the place of yet unannounced tournaments. When a new tournament gets announced it will replace one of the placeholders. These are simulated the same as a Dreamhack with an unannounced player list. This ensures that all of the WCS Points are being accounted for, and prevents giving players 0% or 100% chances prematurely.");
		
		faq += SubSection("What are events?", "Events are things that can happen that help or hurt a player, for example MC winning the GSL is an event for MC, but it can also be an event for JaeDong if it is determined to have a big enough effect on him. There are also grouped events like 'MC wins his next match AND Life wins GSL' is 2 events grouped together so you can see how both things affect the player's chances."+ret+=Section("Events  that will help or hurt "+players.pshorthtml(15)+"'s chances of qualifying for Blizzcon", PlayerEvents(15).Html(DEBUGARG), "player-events-section flush-section")+"You can see these tables on the page for each player, just click on their name. The Score column is just a score for how relevant the event is for the player, Probability is how likely the event or group of events is to occur, Events column shows what happens, Blizzcon Chances is how it affects the player's Blizzcon Chances. Notice that by default it filters for events that are simple and likely. You can use the buttons to change the filters, or type in filters manually. Mouse over the <i class=\"fa fa-info\" title=\"Keywords:HyuN, HyuN-1st-GSL-Code-A-Season-2, HyuN-GSL-Code-A-Season-2-1st, GSL-Code-A-Season-2-1st-HyuN, 1st-GSL-Code-A-Season-2-HyuN, simple, single, self, positive, notor, helps, is-likely\"></i> to see the keywords for each event." );
		
		faq += SubSection("Why isn't the event for HyuN getting 8th place at &lt;Random Tournament&gt; listed?", "WCS Predictor only finds and tallies events that either have a high enough score (how much it CHANGES their Blizzcon chances combined with how likely it is), are an upcoming match, or are a 1st place.");
		faq += SubSection("How do you decide if a match is important or a must win for a player?", "Important matches just means it has a big effect on their Blizzcon Chances. Must Win means that it has a huge effect on their chances.");
		faq += SubSection("How can a player lose 50% Blizzcon Chances for just one match?", "Let's say you have 1 match that determines it all for you, you must win the match to go to Blizzcon. Maybe it's your last match, or it's just the last match that gives you enough points. If you have a 50% chance to win the match, and your chances go to 100% when you win and 0% when you lose, then your current Blizzcon chances are 50%.");
		
		/*faq += SubSection("What is WCS Wars?",
						  "<p>WCS Wars is a combination fantasy league and turn-based strategy game (since everyone does normal fantasy leagues). If someone just wants to do a normal fantasy league then they can just play that and ignore the strategy game.</p>"
						  "<p>WCS Wars is based on the median WCS points as determined by WCS Predictor. You currently start the game with 15,000 WCS Points to buy players with (this starting number will need to be adjusted depending on the season). Each player's cost is their Median WCS Points + a base cost for every player (currently 1000) + some extra points if they have a headband.</p>"
						  "<p>Once you build a team you can then search for matches against other players. You start the match with an Airport to fly in other players, and 2 \"Generic Foreigners\" who can fight and stream on Twitch to get money. You'll need to get money to be able to fly in (or build) the players of your team, or more Generic Foreigners, or Generic Koreans, and their cost will be once again related to their Median WCS Points (what they were at the start of the match), and their damage will also.</p>"
						  "<p>Currently you start each turn with 1 action point for special actions, move actions, build actions, and attack actions. In the future I may make it so that you have different numbers of action points, maybe being able to upgrade your action points per turn.</p>"
						  "<p>The game interface is currently pretty rough, I know.</p>");*/
		
		faq += SubSection("<span id='wcs-wars'>What is WCS Wars?</span>",
			"<p>WCS Wars is a fantasy league based on the median WCS points as determined by WCS Predictor (<a href='http://en.wikipedia.org/wiki/Median'>Wikipedia link for median</a>). You currently start the game with 18,000 WCS Points to buy players with (this starting number will need to be adjusted depending on the season). Each player's value = median_wcs_points + base_cost, where the base_cost is currently 2000. Build a team of up to 5 players, hope that your players win to boost their Median WCS Points, and you can sell them to buy different players. Selling gives you 99% of their worth. There will be a winner each season and the teams will all be reset. Check the Leaderboard section at the bottom of the WCS Wars page!</p>");
		faq += SubSection("Can I gain points in WCS Wars even if the player is already locked in for Blizzcon?",
			"<p>Absolutely! Since WCS Wars value is based on median WCS Points every win helps, as opposed to % Blizzcon Chances which caps out at 100%. Their value can go up from winning a match and gaining more WCS Points, being seeded/invited into a tournament, qualifying for a tournament, a strong player being knocked out, their Aligulac rating going up, even winning in an unrelated tournament could boost their Aligulac rating enough to increase their value.</p>");
		
		//faq += SubSection("What are the Headbands again?", "...");
		
		faq += SubSection("What are branched simulations?", "<p>A branched simulation is when it looks at all the samples in a simulation, finds the samples it wants, and then creates a new simulation based on just those samples with all its own events and statistics.</p>"
						  "<p>So if I make a filter to branch for when Flash wins the first GSL of the year, in that branched simulation he will have a 100% chance to win that GSL and everyone else will have 0% chance, and Flash's Blizzcon chances and AFK Chances will be much higher. However who gets 2nd place in GSL S1 in that example branch would still be up in the air.</p>"
						  "<p>This will allow me to answer people much better when they ask for things like \"What happens if Snute gets 4th place at Dreamhack, Classic loses his next match in GSL, Scarlett wins WCS AM with Polt getting 2nd place, and San wins WCS EU?\", I will be able to run a branched simulation and give them the huge amount of stats that WCS Predictor normally gives for the current situation except with their hypothetical situation.</p>");
		
		ret += Section("Frequently Asked Questions", faq, "faq-section");
		
		/*ret += Section("Feedback", "Just PM me on Team Liquid to give me feedback. I'm mostly looking for feedback on WCS Wars, and the general layout and style of the website. Hopefully WCS Predictor 2015 is a much better looking website than WCS Predictor 2014 was. For WCSP2015 I'm going with a single column style, so that things are grouped together better and more relevant, and it should be easier to read. How do you guys like the header bar at the top, the design for the section boxes with titles, the colors? If you have any ideas to improve WCS Predictor or make it easier to use, now is the best time while I'm rewriting it all. Also more questions for the FAQ are always welcome.","feedback-section");*/
		
		/*string notes;
		string wwnotes = "<p>Some random ideas I'm thinking of doing -</p>"
		//"<p>bo3/bo5 ladder, automated tournaments, map editor (capable of making custom maps like RPGs and tower defense?)</p>"
		"<p>In game money to build units and maybe research upgrades. Maybe you make money by bringing your units to tournaments on the map, or internet cafes to stream?</p>"
		"<p>Maybe the sell value for players should be 95% or 100%? I want to promote trading a bit, but I think 100% might be too much?</p>"
		"<p>Maybe Zerg units do 2 hits per attack so that armor upgrades are better against Zerg?</p>"
		"<p>The game needs to promote a good balance of \"guessing skill\" and \"playing skill\", units with low median wcs points should still be useful because they will be cheaper and build faster but also maybe have different abilities.</p>"
		"<p>In 1 turn you should be able to build something and move something and attack something (this now works with action points)</p>"
		"<p>How big should the maps be? How long should an average match take?</p>"
		"<p>Should there be different buildings you can make? Should there be upgrades? Maybe a building that researches tier 1 and tier 2 abilities, a separate building to research tier 3 abilities, and a building that boosts your economy, that way you can see what the opponent is going for by what buildings they have?</p>"
		"<p>Should there be a scouting unit that can move around the map quickly(there will be fog of war)?</p>";
		notes += SubSection("WCS Wars", wwnotes);
		ret += Section("Notes", notes, "notes-section");*/

		/*ret += Section("Recent Changes",
			"<p>1/5/15 - Added pages for users (click on the usernames in the WCS Wars leaderboard), added graphs to tournament/team/country/race/user pages for the top 10 players of the set.</p>"
			"<p>1/4/15 - WCS Predictor 2015 Website released.</p>"*/
			/*"<p>1/2/15 - Updated FAQ.</p>"
			"<p>12/29/14 - Added Summed Percentage Graphs for Races, Teams, and Countries.</p>"
			"<p>12/19/14 - Show Sim ID #s and Update Events details to the points on all the graphs, just mouse over to see them.</p>"
			"<p>12/18/14 - Restricted graphs to not show last year's stats</p>"
			"<p>12/13/14 - Changed coloring scheme for percentages, added tournament pages, player set pages for races/countries/teams/pids</p>"
			"<p>12/12/14 - Added tournaments to upcoming sections, added Tournaments page, finished Foreigner Hope section</p>"
			"<p>12/6/14 - Added update events to Simulation Info section</p>"
			"<p>12/4/14 - Changed colors to be red tinted instead of blue tinted so it's easier on the eyes, mostly finished player pages, added full player list page</p>"
			"<p>11/4/14 - Added fancy tooltips, AFK Chances, biggest winners/losers with graphs, WCS Point Cutoffs with graph, dropdown navigation, " / *"internet cafes in WCS Wars,"* // * "a bunch of little things and tweaks.</p>"
			"<p>10/19/14 - Made sections collapsable, put top 25 players list table on the front page, made tables flush for top player and buy players, added biggest winners and biggers losers (still need the graphs), added headbands.</p>"
			//"<p>10/18/14 - Action points, cooldowns on attacks, end turn buttons, restricting to 1 of each of your players, money, game chat, in-game alerts, and bug fixes.</p>"
			"<p>10/9/14 - Added this page</p>"*/
			/*, "recent-updates-section");*/
		return ret;
	}
};

class LoginPage : public Page
{
public:
	string err;
	LoginPage() {
		bodyclass = "login-page";
	}

	LoginPage(string error) : err(error) {
	}

	/*virtual string HtmlPrefix() {
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + Header(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		//need a module for HeaderBar/NavBar? also needs to load alerts and stuff so yea it needs to be a global module instead of a function
		//pre += "\n\n" + headerbar.Html();
		//pre += "\n\n";
		//pre += Section("Simulation Info", simheader.Html(), "sim-info-section");
		//pre += "\n\n";
		//pre += current_user.Html() + "\n\n";
		return pre;
	}*/
	
	virtual string InnerHtml()
	{
		string ret;
		if (err.length() > 0) {
			ret += Section("ERROR", err, "account-error-section");
		}
		string edit_profile;
		if (current_user.user_id == 0) edit_profile = "<mdiv>You need to be logged in to edit your profile.</mdiv>";
		else {
			if (postvars["tlname"] && postvars["redditname"] && postvars["twitchname"] && postvars["twittername"]) {
				current_user.tlname = postvars["tlname"].ToString();
				current_user.redditname = postvars["redditname"].ToString();
				current_user.twitchname = postvars["twitchname"].ToString();
				current_user.twittername = postvars["twittername"].ToString();
				current_user.Save();
			}
			string tlname = jsonescape(current_user.tlname,120);
			string redditname = jsonescape(current_user.redditname,120);
			string twitchname = jsonescape(current_user.twitchname,120);
			string twittername = jsonescape(current_user.twittername,120);
			//edit_profile += "<mdiv>"+HtmlEscape(postvars.ToString(),64000)+"</mdiv>";
			edit_profile += "<form class='edit-profile-form' method='post'>";
			edit_profile += "<mdiv>TeamLiquid Username: <input name='tlname' class='tl-username' placeholder='TeamLiquid Username' value='"+ tlname +"'></mdiv>";
			edit_profile += "<mdiv>Reddit Username: <input name='redditname' class='reddit-username' placeholder='Reddit Username' value='"+ redditname +"'></mdiv>";
			edit_profile += "<mdiv>Twitch Username: <input name='twitchname' class='twitch-username' placeholder='Twitch Username' value='" + twitchname + "'></mdiv>";
			edit_profile += "<mdiv>Twitter Username: <input name='twittername' class='twitter-username' placeholder='Twitter Username' value='" + twittername + "'></mdiv>";
			edit_profile += "<mdiv><input type = 'submit' class = 'apply-button' value = 'Apply'></mdiv>";
			edit_profile += "</form>";
		}
		ret += Section("Edit Profile", edit_profile, "edit-profile-section");

		string form;
		if(current_user.user_id==0) form += "You need to login for WCS Wars.<br/>";
		else form += "You are currently logged in as " + current_user.namehtml() + ".<br/>";
		form += "<form class='login-form' action='" + MakeURL("") + "' method='get'><input class='login-username' placeholder='Username'><input class='login-password' type='password' placeholder='Password'><input type='submit' class='login-button' value='Login'></form>";
		if(current_user.user_id==0) ret += Section("Login", form, "login-section");
		else {
			ret += Section("Switch Users", form, "login-section");
		}

		form = "Make a new account.<br/>";
		form += "<form class='create-account-form' action='" + MakeURL("") + "' method='get'><input class='create-username' placeholder='Username'><input class='create-password' type='password' placeholder='Password'><input class='confirm-create-password' type='password' placeholder='Confirm Password'><input type='submit' class='create-account-button' value='Create'></form>";
		ret += Section("Create Account", form, "create-account-section");
		
		ret += Section("Leaderboard", leaderboard.Html(DEBUGARG), "leaderboard-section");
		return ret;
	}
};

class GamePage : public Page
{
public:
	GamePage()
	{
		bodyclass = "game-page";
		title = "WCS Wars 2015";
	}

	virtual string Html(DEBUGARGDEF) {
		GetData(DEBUGARG);
		if (current_user.user_id == 0) {
			return LoginPage().Html(DEBUGARG);
		}
		string ad = "";//can't do ads....
		return HtmlPrefix() + InnerHtml() + ad + HtmlPostfix();
	}
	//virtual string Html() { GetData(DEBUGARG); return HtmlPrefix() + InnerHtml() + HtmlPostfix(); }

	virtual string HtmlPrefix()
	{
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + HeaderBase(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		headerbar.homelink = "page=gamehome";
		headerbar.pagename = "<i class='fa fa-play'></i>WCS Wars";
		headerbar.rightlinkname = "<i class='fa fa-home'></i>WCS Predictor 2015";
		headerbar.rightlinkurl = "";
		pre += "\n\n" + headerbar.Html(DEBUGARG);
		pre += "\n\n";
		//pre += "<audio id=\"beep-sound\" src=\"beep.wav\" preload=\"auto\"></audio>\n\n";
		//pre += Section("Simulation Info", gamesimheader.Html(DEBUGARG), "sim-info-section", true);
		gamesimheader.GetData(DEBUGARG);
		//string age_string=simheader.AgeString();
		//pre += Section("Simulation Info - "+age_string+" - "+ToStringCommas(simheader.samples)+" samples", gamesimheader.Html(DEBUGARG), "sim-info-section", true);
		pre += DefaultSimHeaderSection();
		pre += "\n\n";
		pre += current_user_team.Html(DEBUGARG) + "\n\n";
		pre += wwglobals.Html(DEBUGARG) + "\n\n";
		pre += headbands.ToJSON(DEBUGARG) + "\n\n";
		return pre;
	}
};

class GameHome : public GamePage
{
public:
	GameHome() {
		bodyclass = "game-home";
	}

	virtual string InnerHtml()
	{
		string ret;
		ret += UserTeamSection(current_user_team).Html(DEBUGARG);

		/*if(current_user.user_id<=14 && current_user.user_id>0) {
			ret += Section("Your Current Matches", matchsearches.Html(DEBUGARG) + "<rhr></rhr>" + currentmatches.Html(DEBUGARG), "mymatches-section");
			ret += Section("Start a Match", "<div>"
					   "<button onclick=\"FindMatch('1v1')\"><i class='fa fa-search'></i>Find 1v1 Match</button>"
					   "<button onclick=\"FindMatch('2v2')\"><i class='fa fa-search'></i>Find 2v2 Match</button>"
					   "<button onclick=\"FindMatch('ffa')\"><i class='fa fa-search'></i>Find 4 Player FFA Match</button>"
					   "<button onclick=\"FindMatch('passion')\"><i class='fa fa-search'></i>Find 1v1 Passion of the Cards Match</button>"
					   //"<button onclick=\"FindMatch('coop')\"><i class='fa fa-search'></i>Find Co-op Game</button>"
						"</div>", "matchsearch-section");
		}*/

		ret += Section("Leaderboard", leaderboard.Html(DEBUGARG), "leaderboard-section");
		return ret;
	}
};

class GameStats : public GamePage
{
public:
	GameStats() {
		bodyclass = "game-stats";
	}
	
	virtual string InnerHtml()
	{
		string ret;
		QuickestProfitableTrades quickest_profits;
		GiversTakers givers_takers;
		MostSales most_sales;
		MostBought most_bought;
		UntappedPotential untapped_potential(&most_bought);
		JudgeTiming judge_timing;

		ret += Section("Best Trade", best_worst_trades.BestTrades(DEBUGARG), "best-trade-section");
		ret += Section("Worst Trade", best_worst_trades.WorstTrades(DEBUGARG), "worst-trade-section");
		ret += Section("Quickest Profitable Sale", quickest_profits.Html(DEBUGARG), "quick-profit-section");
		ret += Section("Givers", givers_takers.Givers(DEBUGARG), "givers-section");
		ret += Section("Takers", givers_takers.Takers(DEBUGARG), "takers-section");
		ret += Section("Most Sales", most_sales.Html(DEBUGARG), "most-sales-section");
		ret += Section("Most Bought", most_bought.Html(DEBUGARG), "most-bought-section");
		//ret += Section("Most Sold", "The players that were sold the most.", "most-sold-section");
		ret += Section("Best Possible Trades", players_value_history.Html(DEBUGARG), "best-possible-trades-section");
		ret += Section("Untapped Potential", untapped_potential.Html(DEBUGARG), "untapped-potential-section");
		ret += Section("Best Timing", judge_timing.BestTiming(DEBUGARG), "late-buys-section");
		ret += Section("Worst Timing", judge_timing.WorstTiming(DEBUGARG), "late-buys-section");
		//ret += Section("Premature Sales", "Show users that sold a player a little too early, based on how much more profit they could've made.", "premature-sales-section");
		//ret += Section("Late Sales", "", "premature-sales-section");
		//ret += Section("Premature Buys", "", "late-buys-section");
		//ret += Section("Late Buys", "Show users that bought a player after the player had big gains.", "late-buys-section");
		ret += Section("Most Improved", "Users who gained the most points in the past month.", "most-improved-section");
		ret += Section("Slumpers", "Users who lost the most points in the past month.", "slumpers-section");
		//ret += Section("Leaderboard", leaderboard.Html(DEBUGARG), "leaderboard-section");
		//ret += players_value_history.Html(DEBUGARG);
		return ret;
	}
};

class GameView : public GamePage
{
public:
	GameView() {
		bodyclass = "game-view-page";
	}

	virtual string HtmlPrefix()
	{
		string pre = "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" + HeaderBase(false, bodyclass, title.c_str(), NULL DEBUGARGS);
		headerbar.homelink = "page=gamehome";
		headerbar.pagename = "<i class='fa fa-play'></i>WCS Wars";
		headerbar.rightlinkname = "<i class='fa fa-home'></i>WCS Predictor 2015";
		headerbar.rightlinkurl = "";
		pre += "\n\n" + headerbar.Html(DEBUGARG);
		pre += "\n\n";
		pre += "<audio id=\"beep-sound\" src=\"beep.wav\" preload=\"auto\"></audio>\n\n";
		//pre += Section("Simulation Info", gamesimheader.Html(), "sim-info-section");
		pre += "\n\n";
		pre += current_user.Html(DEBUGARG) + "\n\n";
		pre += wwglobals.Html(DEBUGARG) + "\n\n";
		//pre += headbands.Html() + "\n\n";
		return pre;
	}
	
	virtual string InnerHtml()
	{
		string ret;
		//Match match((uint)vars["matchid"]);
		auto match = CreateMatchObject((uint)vars["matchid"], false DEBUGARGS);
		ret += "\n<script>window.wcs.match=" + match->Html() + ";</script>\n";
		ret += "<div class='game-info' style='width:1200px;margin:auto;'>This text will show the players in the game, and whose turn it is.</div>\n";
		//ret += "<div style='width:1204px;height:125px;margin:auto;overflow:auto;' class='game-chat'>Game Chat</div>\n";
		//ret += "<div style='width:1200px;height:600px;padding:0;margin:auto;clear:both;background-color:red;' class='game-view'><div style='float:left;height:100%;width:900px;background-color:green;overflow:auto;' class='game-table'>Loading (make sure you have javascript enabled)</div><div style='float:right;height:100%;width:300px;background-color:blue;' class='game-rightpanel'><canvas style='width:100%;height:290px;background-color:lightgreen;display:block;' class='game-minimap' width='300' height='290'></canvas><div style='width:100%;height:310px;' class='game-selection'>Select a unit to view its stats.</div></div></div>\n";
		//ret += "<div style='width:100%;height:125px;margin:auto;overflow:auto;' class='game-chat'>Game Chat</div>\n";

		ret += "<div style='width:100%;padding:0;margin:auto;clear:both;' class='game-view'>";
		ret += "<div style='height:400px;width:100%;overflow:auto;' class='game-table'>Loading (make sure you have javascript enabled)</div>";

		ret += "<div style='height:180px;width:100%;background-color:grey;clear:both;' class='game-panel'>";
		ret += "<canvas style='float:left;width:20%;height:100%;display:block;' class='game-minimap'></canvas>";
		ret += "<div style='width:40%;height:100%;margin:auto;float:right;display:inline-block;position:relative;' class='game-chat-container'><div class='game-chat' style='overflow-y:scroll;position:absolute;top:0;left:0;right:0;bottom:2em;margin:0;padding:3px;box-sizing:border-box;'>Game Chat</div><textarea class='game-chat-input' maxlength='1024' style='position:absolute;box-sizing:border-box;height:2em;width:100%;margin:0;padding:3px;left:0;right:0;bottom:0;'></textarea></div>\n";
		ret += "<div style='display:inline-block;width:40%;height:100%;float:right' class='game-selection'>Select a unit to view its stats.</div>";
		ret += "</div>";//end of .game-panel

		ret += "</div>\n";//end of .game-view
		return Section("Match", ret, "game-view-section flush-section");
	}
};

class BuyPlayers : public GamePage
{
public:
	BuyPlayers() {
		bodyclass = "buy-players";
	}

	virtual string InnerHtml()
	{
		string ret;
		ret += players.MedHtml();
		ret += UserTeamSection(current_user_team).Html(DEBUGARG);
		ret += Section("Buy Players", Table("buy-players-table"), "buy-players-section flush-section");
		return ret;
	}
};

class UserPage : public GamePage
{
public:
	UserPage() {
		bodyclass = "game-user";
	}

	virtual string Html(DEBUGARGDEF) { GetData(DEBUGARG); return HtmlPrefix() + InnerHtml() + HtmlPostfix(); }

	virtual string InnerHtml()
	{
		vector<uint> pids;
		string ret;
		User tuser;
		tuser.user_id = (uint)vars["uid"];
		tuser.GetData(DEBUGARG);
		UserTeam tteam(tuser);
		ret += UserTeamSection(tteam).Html(DEBUGARG);
		for (auto &p : tteam.team)
		{
			if (p.active == 0) continue;
			pids.push_back(p.player_id);
		}

		TeamValueHistory tvh((uint)vars["uid"]);
		ret += Section(HtmlEscape(tuser.username,256) + "'s Team History", tvh.Html(DEBUGARG), "team-history-section flush-section");
		ret += Section(HtmlEscape(tuser.username, 256) + "'s Trade History", tvh.TradeHistoryHtml(DEBUGARG), "trade-history-section");
		PlayerSetPage pset;
		pset.pids = pids;
		ret += pset.PlayerSetHtml();
		ret += pset.PlayerSetGraphs();
		ret += Section("Events", PlayerSetEvents(pids).Html(DEBUGARG), "player-set-events-section flush-section");
		return ret;
	}
};

class PagesPage : public Page
{
public:
	PagesPage()
	{
		bodyclass = "pages-page";
	}

	virtual string InnerHtml()
	{
		string ret;
		ret +=
			"<mdiv><a href='" + MakeURL("", true) + "'>WCS Predictor Home</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=gamehome", true) + "'>WCS Wars</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=faq", true) + "'>Help and FAQ</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=players") + "'>Full Player List</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=checkup", true) + "'>Checkup</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=must_wins") + "'>Must Win Players</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=tournaments") + "'>Tournaments</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=countries") + "'>Countries</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=simulations", true) + "'>Simulations</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=branches", true) + "'>Branches</a></mdiv>"
			"<mdiv><a href='" + MakeURL("page=login", true) + "'>Profile</a></mdiv>"
			"<mdiv><a href='http://www.teamliquid.net/forum/starcraft-2/471444-wcs-predictor-2015'>Discussion Thread</a></mdiv>";
		return Section("Pages", ret, "pages-section");
	}
};

class BranchesPage : public Page
{
public:
	BranchesPage()
	{
		bodyclass = "branches-page";
	}

	virtual string InnerHtml()
	{
		string ret;
		SimulationBranches branches;
		string explanation = "<mdiv>These branches show statistics for assumed scenarios, such as a specific player winning a tournament and how it would affect all the stats.</mdiv>";
		ret += Section("Branches", explanation+branches.Html(DEBUGARG), "branches-section");
		return ret;
	}
};

class CheckupPage : public Page
{
public:
	CheckupPage()
	{
		bodyclass = "checkup-page";
	}

	void BuildEventBracket(vector<WebEventGroup> &secures, Player &p, double chances, double minchances, double maxchances, double span, bool hurtful)
	{
		for (auto &eg : events.evgs) {
			if (eg.pid != p.player_id) continue;
			if (eg.type != NORMAL_EVG_TYPE) continue;
			if (eg.events[0].placing == UPCOMING_WIN) continue;//only concrete placements?
			if (eg.events[0].placing == QUALIFY_BLIZZCON) continue;
			if (eg.size() > 2) continue;

			double newchances = (double)eg.hits / (double)(eg.hits + eg.misses);
			if (newchances < minchances || newchances > maxchances) continue;

			bool secure = true;
			for (uint e = 0; e < eg.size(); e++) {
				if (eg.events[e].t_id>100) secure = false;
				if (eg.events[e].p_id != p.player_id && eg.size() != 1) secure = false;
			}
			if (secure) for (uint si = 0; si < secures.size(); si++) {
				auto &s = secures[si];
				if (eg.size() != s.size()) continue;
				uint matches = 0;
				uint sames = 0;
				uint betters = 0;
				uint worses = 0;
				for (uint e = 0; e < eg.size(); e++) {
					if (eg.events[e].t_id == s.events[e].t_id && eg.events[e].p_id == s.events[e].p_id) {
						matches++;
						if (eg.events[e].placing == s.events[e].placing)
							sames++;
						else if ((eg.events[e].placing > s.events[e].placing && s.events[e].p_id == p.player_id) || (eg.events[e].placing < s.events[e].placing && s.events[e].p_id != p.player_id) )
							worses++;
						else
							betters++;
					}
				}
				if (hurtful) {
					uint b = betters;
					betters = worses;
					worses = b;
				}
				if (matches == eg.size() && betters == 0) {
					s = secures.back();
					secures.pop_back();
					si--;
				}
				else if (matches == eg.size() && worses == 0) {
					secure = false;
				}
			}
			if (secure) secures.push_back(eg);
		}
		std::stable_sort(secures.begin(), secures.end(), [span](const WebEventGroup &a, const WebEventGroup &b) {
			if (a.size() != b.size()) return a.size() > b.size();

			double chances_a = (double)a.hits / (double)(a.hits + a.misses);
			double chances_b = (double)b.hits / (double)(b.hits + b.misses);
			if (abs(chances_a - chances_b)<span) {
				double prob_a = (double)(a.hits + a.misses) / (double)simheader.samples;
				double prob_b = (double)(b.hits + b.misses) / (double)simheader.samples;
				if (abs(prob_a-prob_b)>0.01) return prob_a > prob_b;
			}
			return chances_a < chances_b;
		});
	}

	string PlayerCheckup(WebPlayer &p)
	{
		string ret;
		ret += "<mdiv>" + players.phtml(p.player_id) + " " + RenderPercent(p.times_top, simheader.samples) + " with "+ToStringCommas(p.minpoints)+" WCS Points.</mdiv>";
		vector<WebEventGroup> secures;
		vector<WebEventGroup> halves;
		vector<WebEventGroup> eliminations;
		double chances = (double)p.times_top / (double)simheader.samples;
		/*WebEventGroup MostHurtfulEvent;
		MostHurtfulEvent.pid = 0;
		double worstchances = (double)MostHurtfulEvent.hits / (double)(MostHurtfulEvent.hits + MostHurtfulEvent.misses);
		for (auto &eg : events.evgs) {
			if (eg.pid != p.player_id) continue;
			if (eg.type != NORMAL_EVG_TYPE) continue;
			if (eg.events[0].placing == UPCOMING_WIN) continue;//only concrete placements?
			if (eg.events[0].placing == QUALIFY_BLIZZCON) continue;
			if (eg.size() > 2) continue;

			double newchances = (double)eg.hits / (double)(eg.hits + eg.misses);

			if (eg.size() != 1) continue;
			if (newchances < worstchances || (MostHurtfulEvent.pid == 0 && newchances < chances) || (newchances==worstchances && MostHurtfulEvent.events[0].p_id!=p.player_id && eg.events[0].p_id==p.player_id)) {
				worstchances = newchances;
				MostHurtfulEvent = eg;
			}
		}
		//auto &e = MostHurtfulEvent.events[0];
		if (MostHurtfulEvent.pid == 0) {
		}
		else {
			ret += "<mdiv>Most hurtful event - " + EventToHtml(MostHurtfulEvent)+"</mdiv>";
		}*/
		
		if(chances<0.95) BuildEventBracket(secures, p, chances, 0.95, 1.01, 0.05, false);
		if (secures.size()>4) secures.resize(4);
		if(chances<0.5) BuildEventBracket(halves, p, chances, 0.500001, 0.95, 0.01, false);
		if (halves.size()>4) halves.resize(4);
		if (chances>0.001) BuildEventBracket(eliminations, p, chances, -1.0, std::min<double>(chances-0.0001,0.5), 0.0001, true);
		if (eliminations.size()>4) eliminations.resize(4);

		for (auto &s : secures) {
			ret += "<mdiv>To get nearly secured - " + EventToHtml(s) + "</mdiv>";
		}
		for (auto &s : halves) {
			ret += "<mdiv>To get over 50% - " + EventToHtml(s) + "</mdiv>";
		}
		for (auto &s : eliminations) {
			ret += "<mdiv>Hurtful event - " + EventToHtml(s) + "</mdiv>";
		}
		ret += "<mdiv><br/></mdiv>\n";
		return ret;
	}

	string PlayerGroup(string name, uint min_times_top, uint max_times_top)
	{
		string ret;
		uint num = 0;
		for (uint i = 0; i < players.top_players.size(); i++) {
			auto &p = players.top_players[i];
			if (p.times_top >= max_times_top) continue;
			if (p.times_top < min_times_top) break;
			ret += PlayerCheckup(p);
			num++;
		}
		if(num) ret = SubSection(name+" - "+ToString(num)+" Players", ret);
		return ret;
	}

	virtual string InnerHtml()
	{
		string ret;
		players.GetData(DEBUGARG);
		events.GetData(DEBUGARG);
		tournaments.GetData(DEBUGARG);
		upcoming.GetData(DEBUGARG);
		string sec;
		sec += PlayerGroup("Secured Players - 100% Blizzcon Chances", simheader.samples, simheader.samples+1);
		sec += PlayerGroup("Nearly Secured Players - Over 95%", (uint)((double)simheader.samples*0.95), simheader.samples);
		sec += PlayerGroup("Over 20%", (uint)((double)simheader.samples*0.20), (uint)((double)simheader.samples*0.95));
		sec += PlayerGroup("Over 5%", (uint)((double)simheader.samples*0.05), (uint)((double)simheader.samples*0.20));
		sec += PlayerGroup("Over 0%", 1, (uint)((double)simheader.samples*0.05));
		//sec += PlayerGroup("Over 0.1%", (uint)((double)simheader.samples*0.001), (uint)((double)simheader.samples*0.01));
		ret += Section("Checkup", sec, "checkup-section");
		return ret;
	}
};