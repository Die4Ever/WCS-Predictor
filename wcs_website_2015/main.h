
const char * race_colors[] = { "#F2E8B8", "#B8F2B8", "#B8B8F2", "#F2B8B8" };//RPTZ
uint HISTORY_DAYS = 45;
uint SIM_TYPE = 1;
const uint HISTORY_MIN_SIZE = 10000000;
int USE_ALIGULAC = 1;

string MakeURL(string args, bool clear=false)
{
	string ret="/?";
	if (clear == false) {
		if (vars["sim_id"].length()) ret += "sim_id=" + vars["sim_id"].ToString() + "&";
		if (vars["comp_sim_id"].length()) ret += "comp_sim_id=" + vars["comp_sim_id"].ToString() + "&";
		if (vars["comp_days"].length()) ret += "comp_days=" + vars["comp_days"].ToString() + "&";
		if (vars["comp_hours"].length()) ret += "comp_hours=" + vars["comp_hours"].ToString() + "&";
	}
	if(args.length()==0 && ret.length()==2) return string("/");
	if (args.length() == 0) return ret.substr(0, ret.length() - 1);
	return ret + args;
}

enum class Renderer {
	html, bbcode, plain, reddit, eli5
};
class Context {
public:
	Renderer r;
};
Context globalcontext;

string TimestampBB(uint timestamp, bool approx=false)
{
	time_t rawtimestamp = (time_t)timestamp;
	char buffer[128];
	tm * timeinfo;
	timeinfo = gmtime(&rawtimestamp);
	strftime(buffer, 128, "%c GMT", timeinfo);
	string ret = buffer;
	ret = string("[b][blue]")+(approx?"~ " : "")+"[date]"+ret+"[/date][/blue][/b]";
	return ret;
}

uint timestamp_count = 0;

string TimestampHtml(uint timestamp, bool approx=false)
{
	/*time_t rawtimestamp = (time_t)timestamp;
	char buffer[128];
	tm * timeinfo;
	timeinfo=gmtime(&rawtimestamp);
	strftime(buffer,128,"%c GMT",timeinfo);*/
	if(globalcontext.r==Renderer::bbcode) return TimestampBB(timestamp);
	string s_approx="";
	if(approx) s_approx=" approx";
	string ret = "<span class='timestamp"+s_approx+"' data-timestamp='" + ToString(timestamp) + "'></span>";
	if (timestamp_count < 50 || (timestamp_count<1000 && timestamp_count % 100 == 0)) {
		ret += "<script>ConvertTimestamps();</script>";
	}
	timestamp_count++;
	return ret;
}

string DateHtml(uint timestamp)
{
	time_t rawtimestamp = (time_t)timestamp;
	char buffer[128];
	tm * timeinfo;
	//timeinfo = gmtime(&rawtimestamp);
	timeinfo = localtime(&rawtimestamp);
	strftime(buffer, 128, "%b %d", timeinfo);
	string ret;// = buffer;
	ret = str_replace(buffer, " 0", " ");
	if(globalcontext.r==Renderer::bbcode) ret = string("[b]") + ret + "[/b]";
	return ret;
}

string CountdownBB(uint timestamp, bool approx=false)
{
	time_t rawtimestamp = (time_t)timestamp;
	char buffer[128];
	tm * timeinfo;
	timeinfo = gmtime(&rawtimestamp);
	strftime(buffer, 128, "%c GMT", timeinfo);
	string ret = buffer;
	ret=string("[b][red]in ")+(approx?"~ " : "")+"[countdown]"+ret+"[/countdown][/red][/b]";
	return ret;
}

string CountdownHtml(uint timestamp, bool approx=false)
{
	if(globalcontext.r==Renderer::bbcode) return CountdownBB(timestamp, approx);
	string s_approx="";
	if(approx) s_approx=" approx";
	string s = "<span class='countdown"+s_approx+"' data-timestamp='" + ToString(timestamp) + "'></span>";
	if (timestamp_count < 50 || (timestamp_count<1000 && timestamp_count % 100 == 0)) {
		s += "<script>MakeCountdowns();</script>";
	}
	timestamp_count++;
	return s;
	/*string s="<span class='countdown' data-timestamp='"+ToString(timestamp)+"'>";
	int seconds = (int)(timestamp - (uint)time(0));
	int diff=seconds;*/
}

string PercentToString(double p, uint places)
{
	string ret;//="~ ";
	bool neg = false;
	if (p<0.0) {
		neg = true;
		p *= -1.0;
	}
	double mult = std::pow(10.0, (double)places);
	int p1 = (int)p;
	double p2 = std::fmod(p, 1.0)*mult;
	if (p2<0.5 && p2>0.0) p2 = 0.5;
	double decimal = (std::floor(p2 + 0.5) / mult);
	string sdecimal = ToString(decimal);
	if (decimal >= 1.0) sdecimal = "0.999999999999";
	if (neg) ret += "-";
	ret += ToString(p1) + sdecimal.substr(1, places + 1)+"%";
	return ret;
}

string PercentToString(int num, int denom, uint places)
{
	if (num == 0) return "0%";//"~ 0 %";
	if ((num / denom)*denom == num) {
		return /*"~ "+*/ToString(num / denom) + "00%";
	}

	int anum = std::abs(num);
	int adenom = std::abs(denom);

	double p = (double)num / (double)denom *100.0;
	string ret;//="~ ";
	bool neg = false;
	if (p<0.0) {
		neg = true;
		p *= -1.0;
	}
	double mult = std::pow(10.0, (double)places);
	int p1 = (int)(p);
	double p2 = std::fmod(p, 1.0)*mult;
	if (p2<0.5 && p2>0.0) p2 = 0.5;
	double decimal = (std::floor(p2 + 0.5) / mult);
	string sdecimal = ToString(decimal);
	if (decimal >= 1.0) sdecimal = "0.999999999999";
	if (p1 == 100 && anum < adenom) {
		p1 = 99;
		sdecimal = "0.999999999999";
	}
	//p1+=decimal.toString().substr(1,places+2);
	if (neg) ret += "-";
	string s = ToString(p1) + sdecimal.substr(1, places + 1);//ret += ToString(p1) + sdecimal.substr(1, places + 1);
	if (s == "0" && anum > adenom) {
		DEBUGOUT(ERR, s, "num==" << num << ", denom==" << denom << ", places==" << places);
	} else if (s == "100" && anum < adenom) {
		DEBUGOUT(ERR, s, "num==" << num << ", denom==" << denom << ", places==" << places);
	}
	ret += s+"%";
	return ret;
}

string PercentToJson(int num, int denom)
{
	return "[" + ToString(num) + "," + ToString(denom) + "]";
}

string RenderPercent(int num, int denom, double b=-1000.0, int forcecolor=0)
{
	string s = PercentToString(num, denom, 2);
	if (globalcontext.r == Renderer::plain) return s;

	double p = (double)num / (double)denom *100.0;
	string sl = PercentToString(num, denom, 4);
	string s2 = PercentToString(b, 2);
	string ret;
	int color=0;
	if(p>70.0) color=1;
	if(p<30.0) color=-1;
	if(forcecolor!=0) {
		color=forcecolor;
	}
	//sl += " (" + ToString(num) + "/" + ToString(denom) + ")";
	/*if (b == -1000.0) {
		//ret = "<percent title='" + sl + " ("+ToString(num)+"/"+ToString(denom)+")'>" + s + "</percent>";
		ret = "<perc title='" + sl + "'>" + s + "</perc>";
	} else */if(color==0) {
		//ret = "<span class='percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (equal to cutoff)'>" + s + "</span>";
		//ret = "<perc title='" + sl + " (equal to cutoff)'>" + s + "</perc>";
		ret = "<perc title='" + sl + "'>" + s + "</perc>";
	} else if (color==1) {
		//ret = "<span class='percent good-percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (higher than " + s2 + ")'>" + s + "</span>";
		//ret = "<gperc title='" + sl + " (higher than " + s2 + ")'>" + s + "</gperc>";
		ret = "<gperc title='" + sl + "'>" + s + "</gperc>";
	} else if(color==-1) {
		//ret = "<span class='percent bad-percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (lower than " + s2 + ")'>" + s + "</span>";
		//ret = "<bperc title='" + sl + " (lower than " + s2 + ")'>" + s + "</bperc>";
		ret = "<bperc title='" + sl + "'>" + s + "</bperc>";
	}
	if (globalcontext.r == Renderer::bbcode) {
		if (color==0) {
			ret = "[b][blue]"+ret+"[/blue][/b]";
		} else if (color==1) {
			ret = "[b][green]" + ret + "[/green][/b]";
		} else if (color==-1) {
			ret = "[b][red]" + ret + "[/red][/b]";
		}
	}
	return ret;
}

string RenderPercentD(double p, double b=-1000.0, int forcecolor=0)
{
	string s = PercentToString(p, 2);
	if (globalcontext.r == Renderer::plain) return s;

	string sl = PercentToString(p, 4);
	string s2 = PercentToString(b, 2);
	string ret;
	int color=0;
	if(p>70.0) color=1;
	if(p<30.0) color=-1;
	if(forcecolor!=0) {
		color=forcecolor;
	}
	/*if (b == -1000.0) {
		//ret = "<percent title='" + sl + " ("+ToString(num)+"/"+ToString(denom)+")'>" + s + "</percent>";
		ret = "<perc title='" + sl + "'>" + s + "</perc>";
	} else */if(color==0) {
		//ret = "<span class='percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (equal to cutoff)'>" + s + "</span>";
		//ret = "<perc title='" + sl + " (equal to cutoff)'>" + s + "</perc>";
		ret = "<perc title='" + sl + "'>" + s + "</perc>";
	} else if (color==1) {
		//ret = "<span class='percent good-percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (higher than " + s2 + ")'>" + s + "</span>";
		//ret = "<gperc title='" + sl + " (higher than " + s2 + ")'>" + s + "</gperc>";
		ret = "<gperc title='" + sl + "'>" + s + "</gperc>";
	} else if (color==-1) {
		//ret = "<span class='percent bad-percent' title='" + sl + " (" + ToString(num) + "/" + ToString(denom) + ") (lower than " + s2 + ")'>" + s + "</span>";
		//ret = "<bperc title='" + sl + " (lower than " + s2 + ")'>" + s + "</bperc>";
		ret = "<bperc title='" + sl + "'>" + s + "</bperc>";
	}
	if (globalcontext.r == Renderer::bbcode) {
		if (color==0) {
			ret = "[b][blue]"+ret+"[/blue][/b]";
		} else if (color==1) {
			ret = "[b][green]" + ret + "[/green][/b]";
		} else if (color==-1) {
			ret = "[b][red]" + ret + "[/red][/b]";
		}
	}
	return ret;
}

string ChangeArrow(double prob, double old_prob)
{
	//double prob = (double)p.times_top / (double)samples *100.0;
	//double old_prob = (double)p.comp_times_top / (double)comp_samples *100.0;
	double diff = prob - old_prob;
	string arrow_class = "";
	if (diff>0.001) {
		arrow_class = "icon-fixed-width fa fa-arrow-up up-down-icon";
	} else if (diff<-0.001) {
		arrow_class = "icon-fixed-width fa fa-arrow-down up-down-icon";
	} else {
		//arrow_class="up-down-icon neutral-icon";
		return "";
	}
	string sold_percent = PercentToString(old_prob, 2);
	string arrow = "<i class='" + arrow_class + "' title='From " + sold_percent + " ("+PercentToString(diff, 2)+")'></i>";
	return arrow;
}

string IntToPlacing(uint i)
{
	string s;
	if (i % 10 == 1 && i != 11) {
		s = ToString(i) + "st";
	} else if (i % 10 == 2 && i != 12) {
		s = ToString(i) + "nd";
	} else if (i % 10 == 3 && i != 13) {
		s = ToString(i) + "rd";
	} else {
		s = ToString(i) + "th";
	}
	return s;
}

string HeaderBase(bool ajax, string body_classes, const char *title, const char *body_extra DEBUGARGSDEF)
{
	string ret;
	ret += "<!DOCTYPE html><html><head><meta charset=\"utf-8\" />";
	//ret += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=0.5, minimum-scale=0.1, maximum-scale=2.0\">\n";
	ret += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n<meta name=\"viewport\" content=\"initial-scale=0.75, minimum-scale=0.1, maximum-scale=2.0\">\n";
	ret += string("<title>") + title + "</title>";
	ret += "<script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js\"></script>\n";
	ret += "<link rel=\"stylesheet\" href=\"//code.jquery.com/ui/1.10.3/themes/smoothness/jquery-ui.css\">";
	ret += "<script src=\"//ajax.googleapis.com/ajax/libs/jqueryui/1.10.3/jquery-ui.min.js\"></script>\n";
	//ret+= "<link href=\"//netdna.bootstrapcdn.com/font-awesome/3.2.1/css/font-awesome.css\" rel=\"stylesheet\">\n";
	//ret += "<link href=\"//maxcdn.bootstrapcdn.com/font-awesome/4.1.0/css/font-awesome.min.css\" rel=\"stylesheet\">\n";
	ret += "<link href=\"//maxcdn.bootstrapcdn.com/font-awesome/4.2.0/css/font-awesome.min.css\" rel=\"stylesheet\">\n";
	ret += "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://ajax.aspnetcdn.com/ajax/jquery.dataTables/1.9.4/css/jquery.dataTables.css\">\n";
	ret += "<script type=\"text/javascript\" charset=\"utf8\" src=\"http://ajax.aspnetcdn.com/ajax/jquery.dataTables/1.9.4/jquery.dataTables.min.js\"></script>\n";

	//ret+= "<script src=\"http://code.highcharts.com/highcharts.js\"></script>\n";
	ret += "<script type=\"text/javascript\" src=\"http://code.highcharts.com/stock/highstock.js\"></script>\n";
	//ret+= "<script class=\"jsbin\" src=\"//datatables.net/download/build/jquery.dataTables.nightly.js\"></script>\n";
	//ret+= "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>\n";
	if (vars["exporting"]) ret += "<script src=\"http://code.highcharts.com/modules/exporting.js\"></script>\n";

	ret += "<link rel='stylesheet' type='text/css' href='flags.css' />\n";
	ret += "<link rel='stylesheet' type='text/css' href='wcsp2-" VERSION ".css' />\n";
	ret += "<script type=\"text/javascript\" src=\"wcsp2-" VERSION ".js\"></script>\n";
	ret += "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/favicon.ico\" />\n";

	if (body_extra) {
		ret += "</head><body class='" + body_classes + "' " + body_extra + ">\n";
	}
	else {
		ret += "</head><body class='" + body_classes + "'>\n";
	}

	ret += "<script>MakeCountdowns=MakeCountdowns || (function(){}); ConvertTimestamps=ConvertTimestamps || (function(){}); window.wcs=window.wcs || {players:{},player_historiesa:[],team_historiesa:{},teams:{},eventsa:[],foreignhope:{},num_foreigner_hopes:[],headbands:{},upcoming:[],achievements:[],sim:{},comp_sim:{},races:{protoss:{},terran:{},zerg:{},random:{} },countries:{},tournaments:{},currentuser:{},wwglobals:{},cutoffs:{},update_events:[],team_events:[] };</script>\n";

	ret += "<script>\n"
		"  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){\n"
		"  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),\n"
		"  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)\n"
		"  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');\n\n"

		"  ga('create', 'UA-46248146-1', '4ever.tv');\n"
		"  ga('send', 'pageview');\n\n"

		"</script>\n";

	return ret;
}

/*string HeaderBar(const char *header, string homelink, string rightlinkurl, string rightlinkname)
{
	if (rightlinkname == "" && rightlinkurl == "") {
		rightlinkurl = "page=gamehome";
		rightlinkname = "<i class='fa fa-play'></i>WCS Wars";
	}
	string ret;
	string envelope = "<a href='" + MakeURL("page=messages") + "'><i class='fa fa-envelope-o'></i></a>";
	//if (0) envelope = "<a href='" + MakeURL("page=messages") + "'><i class='fa fa-envelope'></i></a>";
	ret += "<h1><a href='" + MakeURL(homelink) + "'>" + header + "</a>";
	ret += "<span style='float:right;dislay:inline-block;'>";
	ret += "<a href='" + MakeURL(rightlinkurl) + "' style='padding-right:1em;'>" + rightlinkname + "</a>";
	ret += envelope;
	ret += "<a href='" + MakeURL("page=settings") + "'><i class='fa fa-cog'></i></a></span>";
	ret += "</h1>\n";
	return ret;
}*/

string Header(bool ajax, string body_classes, const char *title, const char *body_extra DEBUGARGSDEF)
{
	string ret;
	if (ajax == false) {
		ret += HeaderBase(ajax, body_classes, title, body_extra DEBUGARGS);
		//ret += HeaderBar(header, "", "", "");

		//ret += //"<div style='color:#b00000;font-size:150%;font-weight:bold;text-align:center;'>THIS PAGE IS NOT PRETTY YET! THIS PAGE IS NOT DONE YET!</div>"
			//"<div style='font-size:120%;font-weight:bold;text-align:center;display:block;'>This website shows chances of qualifying to Blizzcon, and how different events could affect those chances.</div>"
			//"<div style='font-size:120%;font-weight:bold;text-align:center;display:block;'><a href='http://www.teamliquid.net/forum/viewmessage.php?topic_id=436957'>Discussion Thread and Explanations</a> -- <a href='https://twitter.com/die4everdm'>@Die4EverDM</a></div><br>\n";
	}
	return ret;
}

uint page_start_time = GetMilliCount();
string Footer(bool ajax DEBUGARGSDEF)
{
	string ret;
	if ((BDEBUG && ajax == false) || vars["debug"] == "WTF" || (vars["debug"] == "wtf" && ajax == false))
	{
		ret += "<div class='debuginfo' style='display:none;'>";
		ret += "<br/>\nPage Time " + ToString(GetMilliSpan(page_start_time)) + " ms<br/>\n";
		foreach(CGIVariable*, p, vars)
			ret += HtmlEscape(p->name, 128) + "(" + ToString(p->Hash() % vars.Slots) + ") = " + HtmlEscape(*p, 10000) + "<br/>\n";
		ret += "</div>";
	}

	if (ajax == false) {
		ret += "<div class='footer'><a href='https://twitter.com/Die4EverDM'>@Die4EverDM</a> - <a href='http://www.teamliquid.net/forum/starcraft-2/471444-wcs-predictor-2015'>Team Liquid Discussion Thread</a> - Powered by <a href='http://aligulac.com'>Aligulac</a> and <a href='http://wiki.teamliquid.net/starcraft2/'>Liquipedia</a></div>\n";
		//ret += "<!--[if IE]><div style='color:darkred;font-size:12pt;text-align:center;'>STOP USING INTERNET EXPLORER</div><![endif]-->\n";
		ret += "</body></html>";
	}
	return ret;
}

string Section(string title, string text, string id, bool hidden=false)
{
	string classes=id;
	if (vars["bbcode"].length()) hidden = true;
	const char *s = strchr(id.c_str(), ' ');
	if(s) {
		id=string(id.c_str(), s);
	}
	if(globalcontext.r==Renderer::bbcode) title="[big][b] "+title+" [/b][/big]";
	if(hidden) {
		return "<div class='section closed "+classes+"' id='"+id+"'><h1 class='section-header'>" + title + "<i style='float:right;' class='fa fa-angle-double-down'></i><span style='float:right;padding-right:6px;'><a href='#"+id+"'>#</a></span></h1><div class='inner-section' style='display:none;'>" + text + "</div></div>\n\n";
	}
	return "<div class='section open "+classes+"' id='"+id+"'><h1 class='section-header'>" + title + "<i style='float:right;' class='fa fa-angle-double-up'></i><span style='float:right;padding-right:6px;'><a href='#"+id+"'>#</a></span></h1><div class='inner-section'>" + text + "</div></div>\n\n";
}

string SubSection(string title, string text, string classes = "", string style="")
{//I can just do this with css though? .section .section....
	if (style.length()) style = " style='" + style + "'";
	if(title.length()==0) return "<div class='subsection " + classes + "'"+style+">" + text + "</div>\n";
	if(globalcontext.r==Renderer::bbcode) title="[b] "+title+" [/b]";
	return "<div class='subsection " + classes + "'"+style+"><h2>" + title + "</h2>" + text + "</div>\n";
}

string LeftRight(string left, string right)
{
	//return "<div class='left'>" + left + "</div><div class='right'>" + right + "</div>\n";
	return "<table class='left-right-table'><tr><td class='ltd'>"+left+"</td><td class='rtd'>"+right+"</td></tr></table>\n";
}

string Table(string classes)
{
	return "<table class='" + classes + "'></table>\n";
}

//string LeftRight(vector<string> lsubsections, vector<string> rsubsections, uint lwidth_per, uint rwidth_per);
//string ThreeAcross(vector<string> subsections);

class WebTeam : public Team
{
public:
	uint times_top;
	uint comp_times_top;

	WebTeam()
	{
		times_top = 0;
		comp_times_top = 0;
	}

	string html()
	{
		string h = "<team><a href='"+MakeURL("p=pset&teams=" + ToString(team_id))+"'>" + name.ToString() + "</a></team>";
		if (globalcontext.r == Renderer::bbcode) {
			h = bbcode(h);
		}
		return h;
	}

	string bbcode(string html)
	{
		return ("[url=http://sc2.4ever.tv"+MakeURL("p=pset&teams=" + ToString(team_id)) + "]" + html + "[/url]");
	}
};

class WebPlayer : public Player
{
public:
	//int wcs_points;
	uint times_top;

	uint minpoints;
	uint medianpoints;
	uint meanpoints;
	uint modepoints;
	uint maxpoints;
	uint minpoints_qualify;
	uint maxpoints_notqualify;
	uint comp_times_top;
	uint rank;
	
	uint afk_times_top,afk_samps, afk_times_top_comp,afk_samps_comp;

	WebPlayer()
	{
		minpoints = 0;
		medianpoints = 0;
		meanpoints = 0;
		modepoints = 0;
		ratings[0] = ratings[1] = ratings[2] = ratings[3] = 0;
		comp_times_top = 0;
		rank=0;
		
		afk_times_top=afk_samps=afk_times_top_comp=afk_samps_comp=0;
	}

	char RaceAbbrv()
	{
		if (race == P) return 'p';
		else if (race == T) return 't';
		else if (race == Z) return 'z';
		return 'r';
	}

	const char * RaceName()
	{
		if (race == P) return "Protoss";
		else if (race == T) return "Terran";
		else if (race == Z) return "Zerg";
		return "Random";
	}

	string htmlicons()
	{
		const char *race_icon_class = "r-ico";
		if (race == P) race_icon_class = "p-ico";
		else if (race == T) race_icon_class = "t-ico";
		else if (race == Z) race_icon_class = "z-ico";
		string c = sCountry();
		if (c == "uk") c = "gb";
		string race_letter = "R";
		if (race == P) race_letter = "P";
		else if (race == Z) race_letter = "Z";
		else if (race == T) race_letter = "T";

		string h = "<a href='"+MakeURL("c=" + sCountry()) + "'><img src='/b.gif' class='flag flag-" + c + "'/></a><a href='"+MakeURL("r=" + race_letter) + "'><img src='/b.gif' class='" + race_icon_class + "'/></a>";
		return h;
	}

	string tooltip(uint samples, WebTeam &team)
	{
		//double prob = (double)times_top / (double)samples *100.0;
		//const char *srace = ("Random\0\0Protoss\0Terran\0\0Zerg") + ((int)race * 8);
		string team_name = team.name.ToString();
		if (team_name.length()) team_name += " ";
		//string html = team_name + name.ToString() + "\nRace: " + srace + "\nChances: " + PercentToString(times_top, samples, 2) + "\nRating: " + ToString(ratings[(int)R]) + "\nCountry: " + sCountry() + "\nMin WCS Points: " + ToString(minpoints) + "\nMedian WCS Points: " + ToString(medianpoints);
		string html = team_name + name.ToString();
		return html;
	}

	string html(uint samples, WebTeam &team)
	{
		/*const char *race_class = "random-player-name";
		if (race == P) race_class = "protoss-player-name";
		else if (race == T) race_class = "terran-player-name";
		else if (race == Z) race_class = "zerg-player-name";*/
		const char *headband_class = "";
		if (headband == 1) headband_class = " number1-player-name";
		else if (headband == 2) headband_class = " number2-player-name";
		/*const char *srace = ("Random\0\0Protoss\0Terran\0\0Zerg") + ((int)race * 8);
		double prob = (double)times_top_season[SEASON] / (double)samples *100.0;*/
		//string team_name = team.name.ToString();
		//if (team_name.length()) team_name += " ";

		/*const char *race_icon_class = "race-icon r-icon";
		if (race == P) race_icon_class = "race-icon p-icon";
		else if (race == T) race_icon_class = "race-icon t-icon";
		else if (race == Z) race_icon_class = "race-icon z-icon";

		string c = sCountry();
		if (c == "uk") c = "gb";*/

		//string h = "<span class='player'>" + htmlicons() + "<a title='" + tooltip(samples, team) + "' class='player-name " + race_class + " " + headband_class + "' player-id='" + ToString(player_id) + "' href='"+MakeURL("page=player&player_id=" + ToString(player_id)) + "'>" + name.ToString() + "</a></span>";
		string h = "<player title='"+tooltip(samples,team)+"'>" + htmlicons() + "<a class='pname" + headband_class + "' href='" + MakeURL("pid=" + ToString(player_id)) + "'>" + name.ToString() + "</a></player>";
		if (globalcontext.r == Renderer::bbcode) {
			h = bbcode(samples, h);
		}
		return h;
	}

	string shorthtml(uint samples, WebTeam &team)
	{
		const char *race_abrv = "r";
		if (race == P) race_abrv = "p";
		else if (race == T) race_abrv = "t";
		else if (race == Z) race_abrv = "z";
		const char *headband_class = "";
		if (headband == 1) headband_class = " number1-player-name";
		else if (headband == 2) headband_class = " number2-player-name";
		/*const char *srace = ("Random\0\0Protoss\0Terran\0\0Zerg") + ((int)race * 8);
		double prob = (double)times_top_season[SEASON] / (double)samples *100.0;*/
		//string team_name = team.name.ToString();
		//if (team_name.length()) team_name += " ";

		//string h = "<span class='player'><a title='" + tooltip(samples, team) + "' class='player-name " + race_class + " " + headband_class + "' player-id='" + ToString(player_id) + "' href='"+MakeURL("page=player&player_id=" + ToString(player_id)) + "'>" + name.ToString() + "</a></span>";
		string h = "<player title='"+tooltip(samples, team)+"' r='"+race_abrv+"' c='"+sCountry()+"'><a class='pname" + headband_class + "' href='" + MakeURL("pid=" + ToString(player_id)) + "'>" + name.ToString() + "</a></player>";
		if (globalcontext.r == Renderer::bbcode) {
			h = bbcodeshort(samples, h);
		}
		return h;
	}

	string bbcodeshort(uint samples, string html)
	{
		string bb = "[url=http://sc2.4ever.tv" + MakeURL("pid=" + ToString(player_id)) + "][b]" + html + "[/b][/url]";
		return bb;
	}

	string bbcode(uint samples, string html)
	{
		const char *race_bb = ":r:";
		if (race == P) race_bb = ":p:";
		else if (race == T) race_bb = ":t:";
		else if (race == Z) race_bb = ":z:";
		string bb = ":" + sCountry() + ": " + string(race_bb) + " [url=http://sc2.4ever.tv"+MakeURL("pid=" + ToString(player_id)) + "][b]" + html + "[/b][/url]";
		return bb;
	}
};

class WebTournament
{
public:
	string name;
	string stream;
	uint id;
	uint timestamp;

	WebTournament()
	{
		stream="http://www.twitch.tv/directory/game/StarCraft%20II";
		id = 0;
		timestamp=0;
	}

	WebTournament(string Name, uint t_id) : name(Name)
	{
		stream="http://www.twitch.tv/directory/game/StarCraft%20II";
		id = t_id;
		timestamp=0;
	}

	string html()
	{
		string s = " <a href='" + stream + "' target='_blank'><i class='fa fa-video-camera'></i></a>";
		if (globalcontext.r == Renderer::bbcode) s = s.substr(1);
		string h="<tournament><a href='"+MakeURL("page=tournament&tid=" + ToString(id)) + "'>" + name + "</a>"+s+"</tournament>";
		if (globalcontext.r == Renderer::bbcode) {
			h = bbcode(h);
		}
		return h;
	}

	string bbcode(string html)
	{
		return "[url=http://sc2.4ever.tv"+MakeURL("page=tournament&tid=" + ToString(id)) + "][b]" + html + "[/b][/url]";
	}
};

class ShowMore
{
public:
	bool cut;
	ShowMore() { cut=false; }
	string Start(string name) {
		cut=true;
		if(globalcontext.r==Renderer::bbcode) return "<mdiv><div class='hide-tag'><div class='hide-tag-text'>[spoiler=More "+name+"]</div></mdiv>";
		return "<div class='hide-tag'><div class='hide-tag-text'>+ Show More "+name+" +</div>";
	}
	string StartFullText(string name) {
		cut = true;
		if (globalcontext.r == Renderer::bbcode) return "<mdiv><div class='hide-tag'><div class='hide-tag-text'>[spoiler=" + name + "]</div></mdiv>";
		return "<div class='hide-tag'><div class='hide-tag-text'>+ Show " + name + " +</div>";
	}
	string End() {
		if(cut==false) return string();
		cut=false;
		if(globalcontext.r==Renderer::bbcode) return "<mdiv>[/spoiler]</mdiv></div>";
		return string("</div>");
	}
	~ShowMore()
	{
		if(cut) {
			throw rayexception("unclosed ShowMore tag" DEBUGARGS);
		}
	}
};