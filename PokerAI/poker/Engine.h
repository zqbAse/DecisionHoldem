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
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include "Table.h"
#include <vector>
#include<algorithm>
#include <set>
#include <string.h>
using namespace std;
#define ll unsigned long long

struct river_community {
	unsigned* keys;
	unsigned short* values;
};
struct turn_community {
	unsigned* keys;
	unsigned* values;
};
struct flop_community {
	unsigned* keys;
	unsigned* values;
};

static const ll length_river = 2809475760;
const int river_community_total = 2118760;
const int rivern_rank_divde = 1981;
river_community river_cluster[2652];
//river_community river_cluster2[2652];

static const int length_turn = 305377800;
const int turn_community_total = 230300;
const int turn_rank_divde = 86966 + 2; 
turn_community turn_cluster[2652];
turn_community turn_cluster2[2652];

static const int length_flop = 25989600;
const int flop_community_total = 19600;
const int flop_rank_divde = 1903365 + 3; 
flop_community flop_cluster[2652];
flop_community flop_cluster2[2652];

static const int seven_length = 133784560;
ll seven_keys[seven_length];
unsigned short seven_strengths[seven_length];



static const int length_preflop = 1326;
static const int prim_preflop = 2652; //50*51=2651
static unsigned char preflop_cluster[prim_preflop];


int* preflop_allin[2652];

const int preflop_allin_divde = 1712304;
double preflop_winrate169[169];
class Engine {
public:
	//PokerTable *table;
	unsigned short river20687_1000[20687];
	unsigned short river20687_10000[20687];
	Engine() {
		load();
	}

	void load() {
		ifstream in("/home/zhouqibin/projects/PokerAI/cluster/sevencards_strength.bin", ios::in | ios::binary);
		if (!in) {
			cout << "hand_value file is not exist";
			throw exception();
		}
		in.read((char*)seven_keys, sizeof seven_keys);
		in.read((char*)seven_strengths, sizeof seven_strengths);
		in.close();
		/*int hand_value[length] = { 0 };
		ifstream in("/home/zhouqibin/projects/PokerAI/hand_rank_value52.bin", ios::in | ios::binary);
		if (!in) {
			cout << "hand_value file is not exist";
			throw exception();
		}
		in.read((char*)hand_value, sizeof hand_value);
		in.close();
		ll hand_key[length] = { 0 };
		ifstream in2("/home/zhouqibin/projects/PokerAI/hand_rank_key52.bin", ios::in | ios::binary);
		if (!in2) {
			cout << "hand_key file is not exist";
			throw exception();
		}
		in2.read((char*)hand_key, sizeof hand_key);
		in2.close();
		for (int i = 0; i < length; i++)
			hash_hand[hand_key[i] % prim].push_back(hand_rank(hand_key[i], hand_value[i]));*/

		unsigned preflop_cluster_key[length_preflop] = { 0 };
		unsigned preflop_cluster_value[length_preflop] = { 0 };
		ll ans = 0;
		int wei[1286792];//1286792
		for (int i = 0; i < prim_preflop; i++) preflop_cluster[i] = 169;
		ifstream in3("/home/zhouqibin/projects/PokerAI/cluster/preflop_hand_cluster.bin", ios::in | ios::binary);
		if (!in3) {
			cout << "hand_value file is not exist";
			throw exception();
		}
		in3.read((char*)preflop_cluster_key, sizeof preflop_cluster_key);
		in3.read((char*)preflop_cluster_value, sizeof preflop_cluster_value);
		for (int i = 0; i < length_preflop; i++)
			preflop_cluster[preflop_cluster_key[i]] = preflop_cluster_value[i];
		in3.close();

		{
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					turn_cluster[i * 52 + j].keys = new unsigned[turn_community_total];
					turn_cluster[i * 52 + j].values = new unsigned[turn_community_total];
				}
			ifstream in4("/home/zhouqibin/projects/PokerAI/cluster/turn_hand_potentialaware_100000cluster1326*230300.bin", ios::in | ios::binary);
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					in4.read((char*)turn_cluster[i * 52 + j].keys, sizeof(unsigned) * 230300);
					in4.read((char*)turn_cluster[i * 52 + j].values, sizeof(unsigned) * 230300);
				}
			in4.close();
			{
				memset(wei, 0, sizeof wei), ans = 0;
				for (int i = 0; i < 51; i++)
					for (int j = i + 1; j < 52; j++) {
						int ind = i * 52 + j;
						for (int t = 0; t < 230300; t++) {
							wei[turn_cluster[ind].values[t]]++;
							assert(turn_cluster[ind].values[t] < 100000 && turn_cluster[ind].values[t] >= 0);
						}
					}
				for (int i = 0; i < 100000; i++)
					assert(wei[i] > 0), ans += wei[i];
				assert(ans == 305377800);
			}
		}
		{
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					turn_cluster2[i * 52 + j].keys = new unsigned[turn_community_total];
					turn_cluster2[i * 52 + j].values = new unsigned[turn_community_total];
				}
			ifstream in4("/home/zhouqibin/projects/PokerAI/cluster/turn_hand_potentialaware_10000cluster1326*230300.bin", ios::in | ios::binary);
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					in4.read((char*)turn_cluster2[i * 52 + j].keys, sizeof(unsigned) * 230300);
					in4.read((char*)turn_cluster2[i * 52 + j].values, sizeof(unsigned) * 230300);
				}
			in4.close();
			{
				memset(wei, 0, sizeof wei), ans = 0;
				for (int i = 0; i < 51; i++)
					for (int j = i + 1; j < 52; j++) {
						int ind = i * 52 + j;
						for (int t = 0; t < 230300; t++) {
							wei[turn_cluster2[ind].values[t]]++;
							assert(turn_cluster2[ind].values[t] < 10000 && turn_cluster2[ind].values[t] >= 0);
						}
					}
				for (int i = 0; i < 10000; i++)
					assert(wei[i] > 0), ans += wei[i];
				assert(ans == 305377800);
			}
		}

		{
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					flop_cluster[i * 52 + j].keys = new unsigned[flop_community_total];
					flop_cluster[i * 52 + j].values = new unsigned[flop_community_total];
				}
			ifstream in5("/home/zhouqibin/projects/PokerAI/cluster/flop_hand_lossless_cluster1326*19600.bin", ios::in | ios::binary);//flop_hand_potentialaware_50000cluster1326*19600
			for (int i = 0; i < 51; i++)//flop_hand_lossless_cluster1326*19600.bin
				for (int j = i + 1; j < 52; j++) {
					in5.read((char*)flop_cluster[i * 52 + j].keys, sizeof(unsigned) * 19600);
					in5.read((char*)flop_cluster[i * 52 + j].values, sizeof(unsigned) * 19600);
				}
			in5.close();
			{
				memset(wei, 0, sizeof wei), ans = 0;
				for (int i = 0; i < 51; i++)
					for (int j = i + 1; j < 52; j++) {
						int ind = i * 52 + j;
						for (int t = 0; t < 19600; t++) {
							wei[flop_cluster[ind].values[t]]++;
							assert(flop_cluster[ind].values[t] < 1286792 && flop_cluster[ind].values[t] >= 0);
						}
					}
				for (int i = 0; i < 1286792; i++)
					assert(wei[i] > 0), ans += wei[i];
				assert(ans == 25989600);
			}
		}
		{
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					flop_cluster2[i * 52 + j].keys = new unsigned[flop_community_total];
					flop_cluster2[i * 52 + j].values = new unsigned[flop_community_total];
				}
			ifstream in5("/home/zhouqibin/projects/PokerAI/cluster/flop_hand_potentialaware_50000cluster1326*19600.bin", ios::in | ios::binary);
			for (int i = 0; i < 51; i++)//flop_hand_lossless_cluster1326*19600.bin
				for (int j = i + 1; j < 52; j++) {
					in5.read((char*)flop_cluster2[i * 52 + j].keys, sizeof(unsigned) * 19600);
					in5.read((char*)flop_cluster2[i * 52 + j].values, sizeof(unsigned) * 19600);
				}
			in5.close();
			{
				memset(wei, 0, sizeof wei), ans = 0;
				for (int i = 0; i < 51; i++)
					for (int j = i + 1; j < 52; j++) {
						int ind = i * 52 + j;
						for (int t = 0; t < 19600; t++) {
							wei[flop_cluster2[ind].values[t]]++;
							assert(flop_cluster2[ind].values[t] < 50000 && flop_cluster2[ind].values[t] >= 0);
						}
					}
				for (int i = 0; i < 50000; i++)
					assert(wei[i] > 0), ans += wei[i];
				assert(ans == 25989600);
			}
		}
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				river_cluster[i * 52 + j].keys = new unsigned[river_community_total];
				river_cluster[i * 52 + j].values = new unsigned short[river_community_total];
			}
		ifstream in6("/home/zhouqibin/projects/PokerAI/cluster/river_hand_cluster_lossless20687_1326*2118760.bin", ios::in | ios::binary);
		for (int i = 0; i < 51; i++)
			for (int j = i + 1; j < 52; j++) {
				in6.read((char*)river_cluster[i * 52 + j].keys, sizeof(unsigned) * river_community_total);
				in6.read((char*)river_cluster[i * 52 + j].values, sizeof(unsigned short) * river_community_total);
			}
		in6.close();
		ifstream in61("/home/zhouqibin/projects/PokerAI/cluster/river20687_10000.bin", ios::in | ios::binary);
		in61.read((char*)river20687_10000, sizeof(river20687_10000));
		in61.close();
		ifstream in62("/home/zhouqibin/projects/PokerAI/cluster/river20687_1000.bin", ios::in | ios::binary);
		in62.read((char*)river20687_1000, sizeof(river20687_1000));
		in62.close();
		{
			memset(wei, 0, sizeof wei), ans = 0;
			for (int i = 0; i < 51; i++)
				for (int j = i + 1; j < 52; j++) {
					int ind = i * 52 + j;
					for (int t = 0; t < river_community_total; t++) {
						wei[river_cluster[ind].values[t]]++;
						assert(river_cluster[ind].values[t] < 20687 && river_cluster[ind].values[t] >= 0);
					}
				}
			for (int i = 0; i < 20687; i++)
				assert(wei[i] > 0), ans += wei[i];
			assert(ans == 2809475760);
		}

		int check = 0;
		for (int i = 0; i < prim_preflop; i++)
			if (preflop_cluster[i] >= 0 && preflop_cluster[i] < 169)
				check++;
		assert(prim_preflop - length_preflop == check);
	}

	int get_preflop_cluster(int c1, int c2) { //get preflop card 1326 to 169
		unsigned cnt = c1 * 52 + c2;
		assert(preflop_cluster[cnt] >= 0 && preflop_cluster[cnt] < 169);
		return preflop_cluster[cnt];
	}
	int get_preflop_cluster(unsigned char  hand[]) { //get preflop card 1326 to 169
		unsigned cnt;
		if (hand[0] <= hand[1])
			cnt = hand[0] * 52 + hand[1];
		else
			cnt = hand[1] * 52 + hand[0];
		if (preflop_cluster[cnt] < 0 || preflop_cluster[cnt] >= 169) {
			cout << "preflop_cluster[cnt] <= 0 || preflop_cluster[cnt] > 169:" << preflop_cluster[cnt] << endl;
			throw exception();
		}
		return preflop_cluster[cnt];
	}
	void sortp(unsigned char evalcards[], int len) {
		for (int i = 0; i < len - 1; i++) {
			for (int j = 0; j < len - 1 - i; j++) {
				if (evalcards[j] > evalcards[j + 1]) {
					unsigned temp = evalcards[j];
					evalcards[j] = evalcards[j + 1];
					evalcards[j + 1] = temp;
				}
			}
		}
	}
	unsigned get_flop_cluster(unsigned b1, unsigned b2, unsigned char  com[], bool flag) { //get flop card to 200
		unsigned a1, a2;
		if (b1 > b2) {
			a1 = b2;
			a2 = b1;
		}
		else {
			a1 = b1;
			a2 = b2;
		}
		unsigned char comm[] = { com[0],com[1],com[2] };
		sortp(comm, 3);
		unsigned rank;
		if (flag) {
			rank = find_flop(a1 * 52 + a2, comm[0] * 2704 + comm[1] * 52 + comm[2]);
			assert(rank >= 0 && rank < 1286792);
		}
		else {
			rank = find_flop50000(a1 * 52 + a2, comm[0] * 2704 + comm[1] * 52 + comm[2]);
			assert(rank >= 0 && rank < 50000);
		}
		return rank;
	}
	unsigned get_turn_cluster(unsigned b1, unsigned b2, unsigned char  com[], bool flag) { //get turn card to 200
		unsigned a1, a2;
		if (b1 > b2) {
			a1 = b2;
			a2 = b1;
		}
		else {
			a1 = b1;
			a2 = b2;
		}
		unsigned char comm[] = { com[0],com[1],com[2],com[3] };
		sortp(comm, 4);
		unsigned rank;
		if (flag) {
			rank = find_turn(a1 * 52 + a2, comm[0] * 140608 + comm[1] * 2704 + comm[2] * 52 + comm[3]);
			assert(rank >= 0 && rank < 100000);
		}
		else {
			rank = find_turn10000(a1 * 52 + a2, comm[0] * 140608 + comm[1] * 2704 + comm[2] * 52 + comm[3]);
			assert(rank >= 0 && rank < 10000);
		}
		return rank;
	}
	unsigned get_river_cluster(unsigned b1, unsigned b2, unsigned char com[],bool flag) { //get turn card to 990
		unsigned a1, a2;
		if (b1 > b2) {
			a1 = b2;
			a2 = b1;
		}
		else {
			a1 = b1;
			a2 = b2;
		}
		unsigned char comm[] = { com[0],com[1],com[2],com[3],com[4] };
		sortp(comm, 5);
		unsigned rank = find_river(a1 * 52 + a2, comm[0] * 7311616 + comm[1] * 140608 + comm[2] * 2704 + comm[3] * 52 + comm[4]);
		assert(rank >= 0 && rank < 20687);
		if (flag) {
			assert(river20687_10000[rank] >= 0 && river20687_10000[rank] < 10000);
			return river20687_10000[rank];
		}
		else {
			assert(river20687_1000[rank] >= 0 && river20687_1000[rank] < 1000);
			return river20687_1000[rank];
		}
	}

	unsigned char compute_winner(unsigned char p0_cards[2], unsigned char p1_cards[2], unsigned char community_cards[5]) {
		int strength1 = Maxstrength(p0_cards, community_cards);
		int strength2 = Maxstrength(p1_cards, community_cards);
		if (strength1 < strength2)
			return 0;
		else if (strength1 > strength2)
			return 1;
		else
			return 255;
	}

	inline unsigned short Maxstrength(unsigned char  cards[7]) {
		return find_strength((1ll << cards[0]) + (1ll << cards[1]) + (1ll << cards[2]) + (1ll << cards[3]) + (1ll << cards[4]) + (1ll << cards[5]) + (1ll << cards[6]));
	}
	inline unsigned short Maxstrength(unsigned char  hand[2], unsigned char  community[5]) {
		return find_strength((1ll << hand[0]) + (1ll << hand[1]) + (1ll << community[0]) + (1ll << community[1]) + (1ll << community[2]) + (1ll << community[3]) + (1ll << community[4]));
	}
	inline unsigned short Maxstrength(unsigned char c1, unsigned char c2, unsigned char  community[5]) {
		return find_strength((1ll << c1) + (1ll << c2) + (1ll << community[0]) + (1ll << community[1]) + (1ll << community[2]) + (1ll << community[3]) + (1ll << community[4]));
	}

	inline unsigned short find_strength(ll dest) {//compute 5 card hand strength
		int left = 0, right = seven_length - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (seven_keys[middle] == dest)
				return seven_strengths[middle];
			if (dest > seven_keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand seven cards" << dest << endl;
		throw exception();
	}
	inline unsigned find_turn(unsigned handid, unsigned communityid) {
		int left = 0, right = turn_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (turn_cluster[handid].keys[middle] == communityid)
				return turn_cluster[handid].values[middle];
			if (communityid > turn_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand turn cards" << handid << "," << communityid << endl;
		throw exception();
	}
	inline unsigned find_turn10000(unsigned handid, unsigned communityid) {
		int left = 0, right = turn_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (turn_cluster2[handid].keys[middle] == communityid)
				return turn_cluster2[handid].values[middle];
			if (communityid > turn_cluster2[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand turn cards" << handid << "," << communityid << endl;
		throw exception();
	}
	inline unsigned short find_river(unsigned handid, unsigned communityid) {
		int left = 0, right = river_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (river_cluster[handid].keys[middle] == communityid)
				return river_cluster[handid].values[middle];
			if (communityid > river_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand river cards" << handid << "," << communityid << endl;
		throw exception();
	}

	inline unsigned find_flop(unsigned handid, unsigned communityid) {
		int left = 0, right = flop_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (flop_cluster[handid].keys[middle] == communityid)
				return flop_cluster[handid].values[middle];
			if (communityid > flop_cluster[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand flop cards: " << handid << "," << communityid << endl;
		throw exception();
	}
	unsigned find_flop50000(unsigned handid, unsigned communityid) {
		int left = 0, right = flop_community_total - 1;
		while (left <= right)
		{
			int middle = (left + right) / 2;
			if (flop_cluster2[handid].keys[middle] == communityid)
				return flop_cluster2[handid].values[middle];
			if (communityid > flop_cluster2[handid].keys[middle])
				left = middle + 1;
			else
				right = middle - 1;
		}
		cout << "error hand flop cards: " << handid << "," << communityid << endl;
		throw exception();
	}
};
