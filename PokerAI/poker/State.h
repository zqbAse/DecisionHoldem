################################################################################
#
#   Copyright 2022 The DecisionHoldem Authors，namely，Qibin Zhou，
#   Dongdong Bai，Junge Zhang and Kaiqi Huang. All Rights Reserved.
#
#   Licensed under the GNU AFFERO GENERAL PUBLIC LICENSE
#                 Version 3, 19 November 2007
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License 
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################
#pragma once
#include <algorithm>
#include <math.h>
#include "Table.h"
#include "Engine.h"
using namespace std;

Engine* engine = new Engine();
std::map<unsigned char, int>::iterator actionfind;
map<unsigned char, int> raise_action_chips = { {'l',0},{'d',0},{'n',0},{(unsigned char)1,0},{(unsigned char)2,0},{(unsigned char)3,0},{(unsigned char)4,0},{(unsigned char)6,0},{(unsigned char)8,0},{(unsigned char)5,0},{(unsigned char)7,0} };

char suits[] = "scdh";
char ranks[] = "23456789TJQKA";
class Pokerstate {
public:
	unsigned char has_allin, first_action_of_current_round, player_i_index, betting_stage, n_raises,totraise, ttpot, winplayer;
	PokerTable table;
	unsigned short last_raise, last_bigbet, cur_round_action_num;
	//short flopallin; // use short error ***turnallin***;
	//double preflopallin;

	Pokerstate(PokerTable _table, Engine* _engine = NULL) {
		table = _table;
		//engine = _engine;
	}
	void reset_game_single() {
		reset_game();
		table.deck.reset();
		unsigned char playerscard[2][2], community_cards[] = { table.deck.deal_one_card(), table.deck.deal_one_card(), table.deck.deal_one_card(), table.deck.deal_one_card(), table.deck.deal_one_card() };
		//发双方手牌
		playerscard[0][0] = table.deck.deal_one_card();
		playerscard[0][1] = table.deck.deal_one_card();
		playerscard[1][0] = table.deck.deal_one_card();
		playerscard[1][1] = table.deck.deal_one_card();
		if (playerscard[0][0] > playerscard[0][1]) swap(playerscard[0][0], playerscard[0][1]);
		if (playerscard[1][0] > playerscard[1][1]) swap(playerscard[1][0], playerscard[1][1]);
		winplayer = engine->compute_winner(playerscard[0], playerscard[1], community_cards);//计算第i个采样公共牌的胜负关系

		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_preflop_cluster(playerscard[i]);
			table.players[i].clusters[0] = cl;
		}
		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.players[i].clusters[1] = cl;
			cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.players[i].clusters[1 + 4] = cl;
		}
		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.players[i].clusters[2] = cl;
			cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.players[i].clusters[2 + 4] = cl;
		}
		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.players[i].clusters[3] = cl;
			cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.players[i].clusters[3 + 4] = cl;
		}
	}
	void reset_game() {
		table.reset(); //ayns
		betting_stage = 0;//preflop 0,flop 1, turn 2, river 3, shutdown 4, terminal 5
		reset_betting_round_state();
		has_allin = false;
		table._assign_blinds(50, 100);
		last_bigbet = 100;
		totraise = 0;
		//while (!history.empty()) history.pop_back();
	}
	void move_to_next_player() {
		player_i_index ^= 1;
	}
	void reset_betting_round_state() {
		cur_round_action_num = 0;
		last_raise = 0;//every round last raise
		if (betting_stage > 0)
			player_i_index = 1; //current action player id
		else
			player_i_index = 0;
		n_raises = 0;//current raise time
		first_action_of_current_round = false; // flag check
		ttpot = 1;
		//otpot = 1;
	}
	int find_biggest_bet() {
		int biggest_bet = 0;
		int bet = 0;
		for (int i = 0; i < table.playerlen; i++) {
			bet = table.players[i].n_bet_chips();
			if (biggest_bet < bet)
				biggest_bet = bet;
		}
		return biggest_bet;
	}
	int is_terminal() {
		return betting_stage > 3;
	}
	void compute_payout() {
		for (int i = 0; i < table.playerlen; i++) {
			if (table.players[i].active) {
				assert(table.total_pot == table.total());
				table.players[i].add_chips(table.total_pot);
			}
		}
	}
	void compute_payout(unsigned char winplayer) {
		if (winplayer == 255) {
			table.players[0].n_chips = table.players[0].initial_chips;
			table.players[1].n_chips = table.players[1].initial_chips;
		}
		else if (winplayer <= 1) {
			assert(table.total_pot == table.total());
			table.players[winplayer].add_chips(table.total_pot);
		}
		else
			throw exception();
	}
	int payout(int i) {
		return table.players[i].n_chips - table.players[i].initial_chips;
	}
	bool take_action(unsigned char actionstr) {		//不计算最后输赢，只记录加注流程和check合理性
		cur_round_action_num++;
		if (actionstr == 'l') {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			table.players[player_i_index].call(n_chips_to_call);//last max bet chips - self bet chips = need to call chips
			table.Add_pot(n_chips_to_call);
		}
		else if (actionstr == 'd') {
			betting_stage = 5;
			table.players[player_i_index].active = false;
		}
		else if (actionstr <= 80 || actionstr == 160) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;

			if (actionstr == 7)
				assert(last_raise > 100 || (betting_stage == 0 && last_raise >= 100));
			else if (actionstr == 5) 
				last_raise = pot / 400 * 300;
			else if (actionstr == 3) {
				last_raise = pot / 300 * 100;
				ttpot = 0;
			}
			else
				last_raise = pot * actionstr / 200 * 100;
			if (actionstr >= 8)
				ttpot = 0;
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			assert(last_bigbet == find_biggest_bet());
			table.Add_pot(raise_n_chips);
			n_raises++;
			totraise++;
		}
		else if (actionstr == 'n') {
			int n_chips = table.players[player_i_index].n_chips;
			table.players[player_i_index].raise_to(n_chips);
			table.Add_pot(n_chips);
			has_allin = true;
			last_bigbet = table.players[0].initial_chips;
			n_raises++;
		}
		else {
			cout << "action:" << actionstr << ",not exist the action" << endl;
			throw exception();
		}
		move_to_next_player();
		if (actionstr == 'l' && first_action_of_current_round) {
			if (has_allin)
				betting_stage = 4;
			else
				increment_stage();
			if (betting_stage != 4)
				return true;

		}
		first_action_of_current_round = true;
		return false;
	}
	bool apply_action(unsigned char actionstr) {//执行动作，如果游戏结束自动计算输赢筹码
		cur_round_action_num++;
		if (actionstr == 'l') {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			table.players[player_i_index].call(n_chips_to_call);//last max bet chips - self bet chips = need to call chips
			table.Add_pot(n_chips_to_call);
		}
		else if (actionstr == 'd') {
			betting_stage = 5;
			table.players[player_i_index].active = false;
		}
		else if (actionstr == 'n') {
			int n_chips = table.players[player_i_index].n_chips;
			table.players[player_i_index].raise_to(n_chips);
			table.Add_pot(n_chips);
			has_allin = true;
			last_bigbet = table.players[0].initial_chips;
			n_raises++;
		}
		else if (actionstr <= 80 || actionstr == 160) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;
			if (actionstr == 7)
				assert(last_raise > 100 || (betting_stage == 0 && last_raise >= 100));
			else if (actionstr == 5)
				last_raise = pot / 400 * 300;
			else if (actionstr == 3) {
				last_raise = pot / 300 * 100;
				ttpot = 0;
			}
			else
				last_raise = pot * actionstr / 200 * 100;
			if (actionstr >= 8)
				ttpot = 0;
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			assert(last_bigbet == find_biggest_bet());
			table.Add_pot(raise_n_chips);
			n_raises++;
			totraise++;
		}
		else {
			cout << "action:" << actionstr << ",not exist the action" << endl;
			throw exception();
		}
		move_to_next_player();
		if (betting_stage == 5) {
			compute_payout();
		}
		else if (actionstr == 'l' && first_action_of_current_round) {
			if (has_allin) {
				compute_payout(winplayer);
				betting_stage = 4;
				return false;
			}
			else
				increment_stage();
			if (betting_stage != 4)
				return true;
			/*****************************************************************************/
			else
				compute_payout(winplayer);
			/*****************************************************************************/
		}
		first_action_of_current_round = true;
		return false;
	}
	void increment_stage() {
		betting_stage++;
		if (betting_stage < 4)
			reset_betting_round_state();
		assert(betting_stage <= 4);
	}
	
	int legal_actions(unsigned char* actions) {
		int chips = table.players[player_i_index].n_chips;
		assert(table.total_pot == table.total());
		int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
		int pot = table.total_pot + n_chips_to_call;
		int cur = 0;
		if (n_chips_to_call != 0)
			actions[cur++] = 'd'; //fold
		actions[cur++] = 'l'; //call
		if (has_allin == false) {
			if (betting_stage == 0) {
				if (cur_round_action_num < 3) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && hraise_c - last_raise >= 200 && pot <= 800)
						actions[cur++] = (unsigned char)7; //min raise 
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot

				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 1 2 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (raise_c >= 100 && last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else if (betting_stage == 1) {
				if (ttpot && n_raises == 0 && pot >= 1000 && pot <= 3600) {
					int hraise_c = pot / 300 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)3; //raise 1/3 pot
				}
				if (cur_round_action_num < 2) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && ttpot && hraise_c - last_raise >= 300 && pot <= 6000)
						actions[cur++] = (unsigned char)7; //min raise 

					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot

					int spot = pot / 400 * 300;
					if (spot >= 100 && last_raise <= spot && chips >= n_chips_to_call + spot + 5000 && spot - hraise_c >= 300)
						actions[cur++] = (unsigned char)5; //raise 0.75 pot
				}
				else if (cur_round_action_num == 2) {
					int hraise_c = pot / 200 * 100;
					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 0.5, 1, 2, 4, 8 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else if (betting_stage == 2) {
				if (ttpot && n_raises == 0 && pot >= 1000 && pot <= 3600) {
					int hraise_c = pot / 300 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)3; //raise 1/3 pot
				}
				if (cur_round_action_num < 2) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && ttpot && hraise_c - last_raise >= 300 && pot <= 6000)
						actions[cur++] = (unsigned char)7; //min raise 

					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot

					int spot = pot / 400 * 300;
					if (spot >= 100 && last_raise <= spot && chips >= n_chips_to_call + spot + 5000 && spot - hraise_c >= 300)
						actions[cur++] = (unsigned char)5; //raise 0.75 pot
				}
				else if (cur_round_action_num == 2) {
					int hraise_c = pot / 200 * 100;
					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 0.5, 1, 2, 4, 8 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else {
				if (n_raises < 2) {
					int hraise_c = pot / 200 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4) {
					for (int j = 1; j <= 1; j++) {	// 1, 2 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (raise_c >= 100 && last_raise <= raise_c && chips > n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
				}
			}
			if (chips > 0)
				actions[cur++] = 'n'; //allin
		}
		return cur;
	}
};
class Searchstate {
public:
	unsigned char has_allin, first_action_of_current_round, player_i_index, betting_stage, n_raises, totraise, ttpot, winplayer;
	SearchTable table;
	unsigned short last_raise, last_bigbet, cur_round_action_num;

	Searchstate() {

	}

	void preflopset() {
		assert(betting_stage == 0);
		unsigned char playerscard[2][2], community_cards[5];
		table.deck.reset();
		//设置不安全搜索手牌
		playerscard[0][0] = table.deck.deal_one_card();
		playerscard[0][1] = table.deck.deal_one_card();
		playerscard[1][0] = table.deck.deal_one_card();
		playerscard[1][1] = table.deck.deal_one_card();
		if (playerscard[0][0] > playerscard[0][1]) swap(playerscard[0][0], playerscard[0][1]);
		if (playerscard[1][0] > playerscard[1][1]) swap(playerscard[1][0], playerscard[1][1]);
		//因为当前轮已经固定 只计算一次当前轮的类别
		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_preflop_cluster(playerscard[i]);
			table.clusters[i][betting_stage] = cl;
		}
		for (int i = 0; i < 5; i++)//发剩余公共牌
			community_cards[i] = table.deck.deal_one_card();

		winplayer = engine->compute_winner(playerscard[0], playerscard[1], community_cards);//计算第i个采样公共牌的胜负关系
		for (int i = 0; i < table.playerlen; i++) {
			unsigned cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.clusters[i][1] = cl;
			cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.clusters[i][1 + 3] = cl;
			cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.clusters[i][2] = cl;
			cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.clusters[i][2 + 3] = cl;
			cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
			table.clusters[i][3] = cl;
			cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
			table.clusters[i][3 + 3] = cl;
		}
	}
	void setprivate_publiccards(unsigned char exclude[], int cardid_to_index[]) {
		assert(betting_stage > 0);
		unsigned char playerscard[2][2], community_cards[5];
		//剔除已有公共牌
		table.deck.reset(exclude, betting_stage + 6);
		for (int i = 0; i < betting_stage + 2; i++)
			community_cards[i] = exclude[i + 4];

		//设置不安全搜索手牌
		playerscard[0][0] = exclude[0];
		playerscard[0][1] = exclude[1];
		playerscard[1][0] = exclude[2];
		playerscard[1][1] = exclude[3];
		if (playerscard[0][0] > playerscard[0][1]) swap(playerscard[0][0], playerscard[0][1]);
		if (playerscard[1][0] > playerscard[1][1]) swap(playerscard[1][0], playerscard[1][1]);
		for (int i = 0; i < table.playerlen; i++) 		//当前轮手牌类别
			table.clusters[i][betting_stage] = cardid_to_index[playerscard[i][0] * 52 + playerscard[i][1]];

		for (int i = 0; i < table.deck.cur_index; i++)
			assert(table.deck.cards[i] == exclude[i]);
		for (int i = betting_stage ? betting_stage + 2 : betting_stage; i < 5; i++)//发剩余公共牌
			community_cards[i] = table.deck.deal_one_card();

		winplayer = engine->compute_winner(playerscard[0], playerscard[1], community_cards);//计算第i个采样公共牌的胜负关系

		if (betting_stage == 1) {
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][2] = cl;
				cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][2 + 3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][3 + 3] = cl;
			}
		}
		else if (betting_stage == 2) {
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][3 + 3] = cl;
			}
		}
	}
	
	void setpubliccards(unsigned char exclude[], int cardid_to_index[]) {	//根据采样一组手牌和当前手牌对应的10组公共牌进行计算
		unsigned char playerscard[2][2], community_cards[5];
		if (betting_stage > 0) {	//剔除已有公共牌
			table.deck.reset(exclude, betting_stage + 2);
			for (int i = 0; i < betting_stage + 2; i++)
				community_cards[i] = exclude[i];
		}
		for (int i = 0; i < table.deck.cur_index; i++)
			assert(table.deck.cards[i] == exclude[i]);
		//发双方手牌
		playerscard[0][0] = table.deck.deal_one_card();
		playerscard[0][1] = table.deck.deal_one_card();
		playerscard[1][0] = table.deck.deal_one_card();
		playerscard[1][1] = table.deck.deal_one_card();
		if (playerscard[0][0] > playerscard[0][1]) swap(playerscard[0][0], playerscard[0][1]);
		if (playerscard[1][0] > playerscard[1][1]) swap(playerscard[1][0], playerscard[1][1]);
		if (betting_stage == 0) {	//因为当前轮已经固定 只计算一次当前轮的类别
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_preflop_cluster(playerscard[i]);
				table.clusters[i][0] = cl;
			}
		}
		else {
			for (int i = 0; i < table.playerlen; i++) {		//当前轮手牌类别
				table.clusters[i][betting_stage] = cardid_to_index[playerscard[i][0] * 52 + playerscard[i][1]];
			}
		}
		for (int i = betting_stage ? betting_stage + 2 : 0; i < 5; i++)//发剩余公共牌
			community_cards[i] = table.deck.deal_one_card();
		winplayer = engine->compute_winner(playerscard[0], playerscard[1], community_cards);//计算第i个采样公共牌的胜负关系
		if (betting_stage == 0) {
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][1] = cl;
				cl = engine->get_flop_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][1 + 3] = cl;
				cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][2] = cl;
				cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][2 + 3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][3 + 3] = cl;
			}
		}
		else if (betting_stage == 1) {
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][2] = cl;
				cl = engine->get_turn_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][2 + 3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][3 + 3] = cl;
			}
		}
		else if (betting_stage == 2) {
			for (int i = 0; i < table.playerlen; i++) {
				unsigned cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, true);
				table.clusters[i][3] = cl;
				cl = engine->get_river_cluster(playerscard[i][0], playerscard[i][1], community_cards, false);
				table.clusters[i][3 + 3] = cl;
			}
		}
	}
	void reset_game() {
		table.reset(); //ayns
		betting_stage = 0;//preflop 0,flop 1, turn 2, river 3, shutdown 4, terminal 5
		reset_betting_round_state();
		has_allin = false;
		table._assign_blinds(50, 100);
		last_bigbet = 100;
		totraise = 0;
		//while (!history.empty()) history.pop_back();
	}
	void move_to_next_player() {
		player_i_index ^= 1;
	}
	void reset_betting_round_state() {
		cur_round_action_num = 0;
		last_raise = 0;//every round last raise
		if (betting_stage > 0)
			player_i_index = 1; //current action player id
		else
			player_i_index = 0;
		n_raises = 0;//current raise time
		first_action_of_current_round = false; // flag check
		ttpot = 1;
	}
	int find_biggest_bet() {
		int biggest_bet = 0;
		int bet = 0;
		for (int i = 0; i < table.playerlen; i++) {
			bet = table.players[i].n_bet_chips();
			if (biggest_bet < bet)
				biggest_bet = bet;
		}
		return biggest_bet;
	}
	int is_terminal() {
		return betting_stage > 3;
	}
	void compute_payout() {
		for (int i = 0; i < table.playerlen; i++) {
			if (table.players[i].active) {
				assert(table.total_pot == table.total());
				table.players[i].add_chips(table.total_pot);
			}
		}
	}
	void compute_payout(unsigned char winplayer) {
		if (winplayer == 255) {
			table.players[0].n_chips = table.players[0].initial_chips;
			table.players[1].n_chips = table.players[1].initial_chips;
		}
		else if (winplayer == 0 || winplayer == 1) {
			assert(table.total_pot == table.total());
			table.players[winplayer].add_chips(table.total_pot);
		}
		else
			throw exception();
	}
	int payout(int i) {
		return table.players[i].n_chips - table.players[i].initial_chips;
	}
	bool take_action(unsigned char actionstr) {		//不计算最后输赢，只记录加注流程和check合理性
		cur_round_action_num++;
		if (actionstr == 'l' || actionstr == 'k') {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			table.players[player_i_index].call(n_chips_to_call);//last max bet chips - self bet chips = need to call chips
			table.Add_pot(n_chips_to_call);
		}
		else if (actionstr == 'd') {
			betting_stage = 5;
			table.players[player_i_index].active = false;
		}
		else if (actionstr <= 80 || actionstr == 160) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;
			if (actionstr == 7)
				assert(last_raise >= 100);
			else if (actionstr == 5)
				last_raise = pot / 400 * 300;
			else if (actionstr == 3) {
				last_raise = pot / 300 * 100;
				ttpot = 0;
			}
			else
				last_raise = pot * actionstr / 200 * 100;
			if (actionstr >= 8)
				ttpot = 0;
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			assert(last_bigbet == find_biggest_bet());
			table.Add_pot(raise_n_chips);
			n_raises++;
			totraise++;
		}
		else if (actionstr == 'n') {
			int n_chips = table.players[player_i_index].n_chips;
			table.players[player_i_index].raise_to(n_chips);
			table.Add_pot(n_chips);
			has_allin = true;
			last_bigbet = table.players[0].initial_chips;
			n_raises++;
		}
		else if (raise_action_chips.find(actionstr) != raise_action_chips.end()) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			last_raise = raise_action_chips[actionstr];
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			table.Add_pot(raise_n_chips);
			n_raises++;
		}
		else {
			cout << "action:" << actionstr << ",not exist the action" << endl;
			throw exception();
		}
		move_to_next_player();
		if ((actionstr == 'l' || actionstr == 'k') && first_action_of_current_round) {
			if (has_allin)
				betting_stage = 4;
			else
				increment_stage();
			if (betting_stage != 4)
				return true;

		}
		first_action_of_current_round = true;
		return false;
	}
	bool apply_action(unsigned char actionstr) {//执行动作，如果游戏结束自动计算输赢筹码
		cur_round_action_num++;
		if (actionstr == 'l' || actionstr == 'k') {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			table.players[player_i_index].call(n_chips_to_call);//last max bet chips - self bet chips = need to call chips
			table.Add_pot(n_chips_to_call);
		}
		else if (actionstr == 'd') {
			betting_stage = 5;
			table.players[player_i_index].active = false;
		}
		else if (actionstr == 'n') {
			int n_chips = table.players[player_i_index].n_chips;
			table.players[player_i_index].raise_to(n_chips);
			table.Add_pot(n_chips);
			has_allin = true;
			last_bigbet = table.players[0].initial_chips;
			n_raises++;
		}
		else if (actionstr <= 80 || actionstr == 160) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;
			if (actionstr == 7)
				assert(last_raise >= 100);
			else if (actionstr == 5)
				last_raise = pot / 400 * 300;
			else if (actionstr == 3) {
				last_raise = pot / 300 * 100;
				ttpot = 0;
			}
			else
				last_raise = pot * actionstr / 200 * 100;
			if (actionstr >= 8)
				ttpot = 0;
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			assert(last_bigbet == find_biggest_bet());
			table.Add_pot(raise_n_chips);
			n_raises++;
			totraise++;
		}
		else if (raise_action_chips.find(actionstr) != raise_action_chips.end()) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			last_raise = raise_action_chips[actionstr];
			int raise_n_chips = last_raise + n_chips_to_call;
			last_bigbet += last_raise;
			table.players[player_i_index].raise_to(raise_n_chips);
			table.Add_pot(raise_n_chips);
			n_raises++;
		}
		else {
			cout << "action:" << actionstr << ",not exist the action" << endl;
			throw exception();
		}
		move_to_next_player();
		if (betting_stage == 5) {
			compute_payout();
		}
		else if ((actionstr == 'l' || actionstr == 'k') && first_action_of_current_round) {
			if (has_allin) {
				compute_payout(winplayer);
				betting_stage = 4;
				return false;
			}
			else
				increment_stage();
			if (betting_stage != 4)
				return true;
			/*****************************************************************************/
			else
				compute_payout(winplayer);
			/*****************************************************************************/
		}
		first_action_of_current_round = true;
		return false;
	}
	void increment_stage() {
		betting_stage++;
		if (betting_stage < 4)
			reset_betting_round_state();
		assert(betting_stage <= 4);
	}
	int legal_actions_river(unsigned char* actions) {
		assert(betting_stage == 3);
		int chips = table.players[player_i_index].n_chips;
		assert(table.total_pot == table.total());
		int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
		int pot = table.total_pot + n_chips_to_call;
		int cur = 0;
		if (n_chips_to_call != 0)
			actions[cur++] = 'd'; //fold
		actions[cur++] = 'l'; //call
		if (has_allin == false) {
			if (n_raises == 0 && pot >= 1000 && pot <= 4800) {
				int hraise_c = pot / 300 * 100;
				if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
					actions[cur++] = (unsigned char)3; //raise 1/3 pot
			}
			if (cur_round_action_num < 2) {
				int hraise_c = pot / 200 * 100;
				if (n_raises == 1 && ttpot && hraise_c - last_raise >= 300 && pot <= 8000)
					actions[cur++] = (unsigned char)7; //min raise 

				if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 2000)
					actions[cur++] = (unsigned char)1; //raise 0.5 pot

				int spot = pot / 400 * 300;
				if (spot >= 100 && last_raise <= spot && chips >= n_chips_to_call + spot + 4000 && spot - hraise_c >= 300)
					actions[cur++] = (unsigned char)5; //raise 0.75 pot
			}
			else if (cur_round_action_num == 2) {
				int hraise_c = pot / 200 * 100;
				if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 4000)
					actions[cur++] = (unsigned char)1; //raise 0.5 pot
			}
			if (cur_round_action_num < 4)
				for (int j = 1; j <= 2; j++) {		// 0.5, 1, 2, 4, 8 pot
					int raise_c = pot * (int)pow(2, j) / 200 * 100;
					if (last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 4000) {
						actions[cur++] = (unsigned char)((int)pow(2, j));
					}
				}
			if (chips > 0)
				actions[cur++] = 'n'; //allin
		}
		return cur;
	}
	int legal_actions(unsigned char* actions) {
		int chips = table.players[player_i_index].n_chips;
		assert(table.total_pot == table.total());
		int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
		int pot = table.total_pot + n_chips_to_call;
		int cur = 0;
		if (n_chips_to_call != 0)
			actions[cur++] = 'd'; //fold
		actions[cur++] = 'l'; //call
		if (has_allin == false) {
			if (betting_stage == 0) {
				if (cur_round_action_num < 3) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && hraise_c - last_raise >= 200 && pot <= 800)
						actions[cur++] = (unsigned char)7; //min raise 
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 1 2 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (raise_c >= 100 && last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else if (betting_stage == 1) {
				if (ttpot && n_raises == 0 && pot >= 1000 && pot <= 3600) {
					int hraise_c = pot / 300 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)3; //raise 1/3 pot
				}
				if (cur_round_action_num < 2) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && ttpot && hraise_c - last_raise >= 300 && pot <= 6000)
						actions[cur++] = (unsigned char)7; //min raise 

					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot

					int spot = pot / 400 * 300;
					if (spot >= 100 && last_raise <= spot && chips >= n_chips_to_call + spot + 5000 && spot - hraise_c >= 300)
						actions[cur++] = (unsigned char)5; //raise 0.75 pot
				}
				else if (cur_round_action_num == 2) {
					int hraise_c = pot / 200 * 100;
					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 0.5, 1, 2, 4, 8 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else if (betting_stage == 2) {
				if (ttpot && n_raises == 0 && pot >= 1000 && pot <= 3600) {
					int hraise_c = pot / 300 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)3; //raise 1/3 pot
				}
				if (cur_round_action_num < 2) {
					int hraise_c = pot / 200 * 100;
					if (n_raises == 1 && ttpot && hraise_c - last_raise >= 300 && pot <= 6000)
						actions[cur++] = (unsigned char)7; //min raise 

					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot

					int spot = pot / 400 * 300;
					if (spot >= 100 && last_raise <= spot && chips >= n_chips_to_call + spot + 5000 && spot - hraise_c >= 300)
						actions[cur++] = (unsigned char)5; //raise 0.75 pot
				}
				else if (cur_round_action_num == 2) {
					int hraise_c = pot / 200 * 100;
					if (last_raise <= hraise_c && chips > n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4)
					for (int j = 1; j <= 2; j++) {		// 0.5, 1, 2, 4, 8 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (last_raise <= raise_c && chips >= n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
			}
			else {
				if (n_raises < 2) {
					int hraise_c = pot / 200 * 100;
					if (hraise_c >= 100 && last_raise <= hraise_c && chips >= n_chips_to_call + hraise_c + 5000)
						actions[cur++] = (unsigned char)1; //raise 0.5 pot
				}
				if (cur_round_action_num < 4) {
					for (int j = 1; j <= 1; j++) {	// 1, 2 pot
						int raise_c = pot * (int)pow(2, j) / 200 * 100;
						if (raise_c >= 100 && last_raise <= raise_c && chips > n_chips_to_call + raise_c + 5000) {
							actions[cur++] = (unsigned char)((int)pow(2, j));
						}
					}
				}
			}
			if (chips > 0)
				actions[cur++] = 'n'; //allin
		}
		return cur;
	}
	
	int getraisechip(unsigned char actionstrt) {
		if (actionstrt == 'l' || actionstrt == 'd')
			throw exception();
		if (actionstrt == 'n')
			return table.players[player_i_index].n_chips;

		int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
		int pot = table.total_pot + n_chips_to_call;
		int chips = table.players[player_i_index].n_chips;
		int returnchips;
		if (actionstrt == 7)
			returnchips = last_raise;
		else if (actionstrt == 5)
			returnchips = pot / 400 * 300;
		else if (actionstrt == 3)
			returnchips = pot / 300 * 100;
		else
			returnchips = pot * actionstrt / 200 * 100;

		return returnchips;
	}
	bool check_action(unsigned char actionstrt) {
		if (actionstrt <= 80 || actionstrt == 160) {
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;
			int returnchips;
			if (actionstrt == 7)
				returnchips = last_raise;
			else if (actionstrt == 5)
				returnchips = pot / 400 * 300;
			else if (actionstrt == 3)
				returnchips = pot / 300 * 100;
			else
				returnchips = pot * actionstrt / 200 * 100;

			int raise_n_chips = returnchips + n_chips_to_call;
			if (table.players[player_i_index].n_chips <= raise_n_chips)
				return false;
		}
		return true;
	}
	bool check_raise_equal(unsigned char nodeactionstr, int human_raisechips) {
		actionfind = raise_action_chips.find(nodeactionstr);
		if (actionfind == raise_action_chips.end()) {
			cout << "haven't the action char:" << nodeactionstr << endl;
			throw exception();
		}
		if (raise_action_chips[nodeactionstr] == 0) {
			if (nodeactionstr == 'l' || nodeactionstr == 'n')
				return false;
			int raise_chipsnumber;
			int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
			int pot = table.total_pot + n_chips_to_call;
			if (nodeactionstr == 7)
				raise_chipsnumber = last_raise;
			else if (nodeactionstr == 5)
				raise_chipsnumber = pot / 400 * 300;
			else if (nodeactionstr == 3)
				raise_chipsnumber = pot / 300 * 100;
			else
				raise_chipsnumber = pot * nodeactionstr / 200 * 100;

			return human_raisechips == raise_chipsnumber;
		}
		else
			return false;
	}
	bool check_raise_errorlow100(unsigned char nodeactionstr, int human_raisechips) {
		if (nodeactionstr == 'l' || nodeactionstr == 'n')
			return false;
		int raise_chipsnumber;
		int n_chips_to_call = last_bigbet - table.players[player_i_index].n_bet_chips();
		int pot = table.total_pot + n_chips_to_call;
		if (nodeactionstr == 7)
			raise_chipsnumber = last_raise;
		else if (nodeactionstr == 5)
			raise_chipsnumber = pot / 400 * 300;
		else if (nodeactionstr == 3)
			raise_chipsnumber = pot / 300 * 100;
		else
			raise_chipsnumber = pot * nodeactionstr / 200 * 100;
		raise_chipsnumber = raise_chipsnumber - human_raisechips;
		if (raise_chipsnumber < 0)
			raise_chipsnumber = -raise_chipsnumber;
		return raise_chipsnumber <= 100;
	}
};
