#define RAYHEADERSMAIN
#define VERSION "0.58"
#define DEBUGFILENAME "../debugout-" VERSION ".txt"
#define WEBSITE 1
#ifdef _DEBUG
#define DEBUGLEVEL INFO//CALLSRETURNS//ERR
#define DEBUGFILELEVEL INFO
#else
#define DEBUGLEVEL WARN//CALLSRETURNS//ERR
#define DEBUGFILELEVEL WARN
#endif
//#define DEBUGFILENAME "../debugout.txt"

#include <RayCGI2.h>

#include "../sc2_common.h"
#include "main.h"
#include "web_modules.h"
#include "wcs_wars.h"
#include "pages.h"

int main(int argc, char *argv[])
{
	/*for (int i = -200; i < 2050; i+=11) {
		std::cout << PercentToString(i, 71, 2) << "\n";
	}
	return 0;*/

	DEBUGARG;
	//DEBUGOUT(FATAL,"test","test");

	for (int i = 1; i < argc; i++)
		raycgi.ReadVars(argv[i], vars);
	if (vars["request"] == "quick") {
		std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf blablabla";
		return 0;
	}

	globalcontext.r = Renderer::html;
	string hostname;
	const char *h = getenv("HTTP_HOST");
	if (h) hostname = h;
	if (strstr(hostname.c_str(), "www.")) {
		hostname = strstr(hostname.c_str(), "www.") + 4;
	}
	if (hostname.length() == 0) {
		hostname = "console";
		vars["user"]="die4ever";
		vars["pw"]="733ebb72f889aca95dd519de108809a9";
	}
	vars["hostname"] = hostname;
	if (hostname == "console") {
		std::cout << "enter some params...\n";
		string sline;
		getline(std::cin, sline);
		raycgi.ReadVars(sline.c_str(), vars);
		globalcontext.r = Renderer::plain;
	}

	if ((uint)vars["history_days"]>0) {
		HISTORY_DAYS = (uint)vars["history_days"];
	}

	if (vars["bbcode"] == "1") globalcontext.r = Renderer::bbcode;
	if (vars["redditcode"] == "1") globalcontext.r = Renderer::reddit;
	if (vars["eli5"] == "1") globalcontext.r = Renderer::eli5;
	if (vars["plain"] == "1") globalcontext.r = Renderer::plain;
	if (vars["html"] == "1") globalcontext.r = Renderer::html;

	try {
#ifdef WIN32
		mysqlcon.ConnectToMysql("../sc2_mysqlcreds.txt");
#else
		mysqlcon.ConnectToMysql("../sc2web_mysqlcreds.txt");
#endif
		mysqlcon.StartTransaction();
	} catch (rayexception rex) {
		if (hostname != "console")  {
			std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf Could not connect to database!";
			return 1;
		}
	}

	if(!vars["request"]) {
		if(!vars["page"] && vars["pid"])
			vars["page"]="player";
		if((!vars["page"]) && vars["p"])//overload the p var? or change to a different letter? use d for dummy display name, make sure to put it first?
			vars["page"]=vars["p"].ToString();
		if(!vars["page"]) {
			if(vars["c"] || vars["r"]) vars["page"]="pset";
		}
		if(vars["page"]=="player" && vars["player_id"].ToString().length()>0) {
			vars["pid"]=vars["player_id"].ToString();
		}
	}
	if ((!vars["page"]) && (!vars["request"]))
		vars["page"] = "home";

	uint allowed = CheckLogin(vars["user"], vars["pw"], vars["session"], vars["create"], vars["newpassword"]);
	/*if (allowed == 0) {
		std::cout << LoginPage().Html();
		return 0;
	}*/
	if (allowed==0 && vars["create"]) {
		string res;
		if (vars["user"].ToString().length() < 3) {
			res = LoginPage("Username must be at least 3 characters long!").Html(DEBUGARG);
		} else if (vars["user"].ToString().length()>=32) {
			res = LoginPage("Username must be less than 32 characters long!").Html(DEBUGARG);
		} else {
			res = LoginPage("Username " + HtmlEscape(vars["user"].ToString(), 64) + " is taken!").Html(DEBUGARG);
		}
		std::cout << res;
	}
	string request = vars["request"].ToString();
	string page = vars["page"].ToString();
	if(page=="p") page="player";

	// //if(current_user.user_id>1 && page.length()>0 && request.length()==0) DEBUGOUT(WARN, current_user.user_id, current_user.username<<", "<<page<<", "<<request<<",vars - "<<vars.ToString());

	try {
		if (request.length()) {
			string res;
			if (request == "BuyPlayer") {
				res = BuyPlayer((uint)vars["id"], (uint)vars["cost"]);
			} else if (request == "SellPlayer") {
				res = SellPlayer((uint)vars["id"], (uint)vars["cost"]);
			} else if (request == "ResetPlayers") {
				res = ResetPlayers();
			} else if (request == "FindMatch") {
				res = StartFindMatch(vars["format"].ToString());
			} else if (request == "IssueOrder") {
				auto match = CreateMatchObject((uint)vars["matchid"], true DEBUGARGS);
				res = match->IssueOrder(vars["order"].ToString(), (uint)vars["myunit_id"] DEBUGARGS);
			} else if (request == "UpdateGameState") {
				auto match = CreateMatchObject((uint)vars["matchid"], false DEBUGARGS);
				res = "{\"success\":1,\"gamestate\":" + match->UpdatedGameState() + "}";
			} else if (request == "postChatMessage") {
				auto match = CreateMatchObject((uint)vars["matchid"], false DEBUGARGS);
				res = match->PostChatMessage(vars["message"].ToString());
			} else {
				//cout << "Content-Type: application/json; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf";
				res = "{\"foo\":\"bar\", \"success\":0,\"error\":\"unknown request\"}";
			}
			if (strstr(res.c_str(), "error")) {
				DEBUGOUT(WARN, current_user.user_id, current_user.username << ", " << request << ",vars - " << vars.ToString() << "\n;;;\n res == "<<res);
			}
			std::cout << "Content-Type: application/json; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf" << res;
		} else if (page.length()) {
			//cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf";
			string res;
			if (page == "home") {
				res = HomePage().Html(DEBUGARG);
			} else if(page=="players") {
				res = PlayerListPage().Html(DEBUGARG);
			} else if(page=="pset") {
				res = PlayerSetPage().Html(DEBUGARG);
			} else if(page=="tournaments") {
				res = TournamentsPage().Html(DEBUGARG);
			} else if(page=="tournament") {
				res = TournamentPage().Html(DEBUGARG);
			} else if(page=="player") {
				res = PlayerPage().Html(DEBUGARG);
			} else if(page=="faq") {
				res = HelpPage().Html(DEBUGARG);
			} else if (page == "gamehome") {
				res = GameHome().Html(DEBUGARG);
			} else if (page == "gameview") {
				res = GameView().Html(DEBUGARG);
			} else if (page == "gamestats") {
				res = GameStats().Html(DEBUGARG);
			} else if (page == "buyplayers") {
				res = BuyPlayers().Html(DEBUGARG);
			} else if(page=="settings" || page=="login") {
				res = LoginPage().Html(DEBUGARG);
			} else if(page=="simulations") {
				res = SimulationsPage().Html(DEBUGARG);
			} else if(page=="countries") {
				res = CountriesPage().Html(DEBUGARG);
			} else if (page == "must_wins") {
				res = MustWinPlayersPage().Html(DEBUGARG);
			} else if (page == "user") {
				res = UserPage().Html(DEBUGARG);
			} else if (page == "pages") {
				res = PagesPage().Html(DEBUGARG);
			}
			else if (page == "branches") {
				res = BranchesPage().Html(DEBUGARG);
			} else if (page == "checkup") {
				res = CheckupPage().Html(DEBUGARG);
			} else {
				res = Page().Html(DEBUGARG);
			}
			std::cout << res;
		} else {
			std::cout << Page().Html(DEBUGARG);
		}
	} catch(rayexception rex) {
		std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf Error!<br/>\n";
		if(current_user.user_id==1) std::cout << rex.what(DEBUGARG)<<"\n";
		return 1;
	}
	
	try {
		matchsearches.CheckMakeMatches();
	} catch(rayexception rex) {
		std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n\x0ef\x0bb\x0bf Error!<br/>\n";
		if(current_user.user_id==1) std::cout << rex.what(DEBUGARG)<<"\n";
		return 1;
	}

	if (hostname == "console") {
		system("pause");
	}
	return 0;
}
