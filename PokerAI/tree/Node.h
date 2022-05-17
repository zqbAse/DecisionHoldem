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
#include <assert.h>
#include <exception>
using namespace std;
struct strategy_nodes {		//info set node 
	int action_len = 0;
	unsigned char actionstr;
	int regret, averegret[3];
	strategy_nodes* actions;
	strategy_nodes() :action_len(0), regret(0) {}
	strategy_nodes* findnode(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return (actions + i);
		}
		throw std::exception();
	}
	int findindex(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return i;
		}
		throw std::exception();
	}
	void init_child(unsigned char action_str[], int _action_len) {

		actions = new strategy_nodes[_action_len]();
		for (int i = 0; i < _action_len; i++)
			actions[i].actionstr = action_str[i];
		action_len = _action_len;

	}
	void init_chance_node(int initlen) {
		actions = new strategy_nodes[initlen]();
		action_len = initlen;
	}
};
void calculate_oppstrategy(strategy_nodes* oriregret, int len, double sigma[]) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i].regret;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
void calculate_strategy(strategy_nodes* oriregret, int len, double sigma[], int idx) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i].averegret[idx];
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
struct strategy_node {		//info set node 
	int action_len = 0;
	unsigned char actionstr;
	int regret, averegret;
	strategy_node* actions;
	strategy_node() :action_len(0), regret(0), averegret(0) {}
	strategy_node* findnode(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr  == action)
				return (actions + i);
		}
		throw std::exception();
	}
	int findindex(unsigned char action) {
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return i;
		}
		throw std::exception();
	}
	void init_child(unsigned char action_str[], int _action_len) {

		actions = new strategy_node[_action_len]();
		for (int i = 0; i < _action_len; i++)
			actions[i].actionstr = action_str[i];
		action_len = _action_len;

	}
	void init_chance_node(int initlen) {
		actions = new strategy_node[initlen]();
		action_len = initlen;
	}
};
void calculate_strategy(strategy_node* oriregret, int len, double sigma[]) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = oriregret[i].regret;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
double calculate_strategy(strategy_node* nodes, int len, int idx) {
	assert(len != 0);
	if (nodes[idx].averegret <= 0)
		return 0;
	double regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = nodes[i].averegret;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / len;
	}
	if (regret[idx] > 0)
		return regret[idx] / sum;
	else
		return 0;
}

struct subgame_node {		//info set node
	bool frozen, leaf;	//leaf:深度受限叶节点，frozen：冻结这个节点策略不更新
	int action_len = 0;
	unsigned char actionstr ;
	double regret;
	double ave_strategy;
	subgame_node* actions ;
	strategy_nodes* leafnode ;
	//double* expolitvalues;
	subgame_node() :action_len(0), frozen(false), leaf(false), actions(NULL), regret(0), ave_strategy(0) {}

	subgame_node* findnode(unsigned char action) {
		if (action == 'k')
			action = 'l';
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return (actions + i);
		}
		cout << "not this actions:" << action << endl;
		throw exception();
	}
	bool findnodeexist(unsigned char action) {
		if (action == 'k')
			action = 'l';
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return true;
		}
		return false;
	}
	int findindex(unsigned char action) {
		if (action == 'k')
			action = 'l';
		for (int i = 0; i < action_len; i++) {
			if (actions[i].actionstr == action)
				return i;
		}
		cout << "not this actions:" << action << endl;
		throw exception();
	}
	void init_child(unsigned char action_str[], int _action_len) {
		//if (action_len == 0) {
		actions = new subgame_node[_action_len]();
		for (int i = 0; i < _action_len; i++)
			actions[i].actionstr = action_str[i];
		action_len = _action_len;
		//}
	}
	void init_chance_node(int initlen) {
		//if (action_len == 0) {
			actions = new subgame_node[initlen]();
			//expolitvalues = new double[initlen] {0};
			action_len = initlen;
		//}
	}
};
void calculate_avestrategy(subgame_node* node,int len, double sigma[]) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = node[i].ave_strategy;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
void calculate_accstrategy(subgame_node* node, int len, double sigma[]) {
	assert(len != 0);
	int regret[12];
	for (int i = 0; i < len; i++)
		regret[i] = node[i].regret;
	double sum = 0;
	for (int i = 0; i < len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum > 0)
		for (int i = 0; i < len; i++)
			if (regret[i] > 0)
				sigma[i] = regret[i] / sum;
			else
				sigma[i] = 0;
	else
		for (int i = 0; i < len; i++)
			sigma[i] = 1.0 / len;
}
double calculate_accstrategy_action(subgame_node* node, int biasid, int index) {
	assert(node->action_len != 0);
	double regret[12];
	for (int i = 0; i < node->action_len; i++)
		regret[i] = node->actions[i].regret;
	if (biasid == 1) {
		if (node->actions[0].actionstr == 'd' && regret[0] > 0)
			regret[0] *= 5;
	}
	else if (biasid == 2) {
		if (node->actions[0].actionstr == 'l') {
			if (regret[0] > 0)
				regret[0] *= 5;
		}
		else if (node->actions[0].actionstr == 'l') {
			if (regret[1] > 0)
				regret[1] *= 5;
		}
		else
			throw exception();
	}
	else if (biasid == 3) {
		for (int i = 0; i < node->action_len; i++)
			if (regret[i] > 0) {
				if (node->actions[i].actionstr != 'l' && node->actions[i].actionstr != 'd')
					regret[i] *= 5;
			}
	}
	else if (biasid == 4) {
		for (int i = 0; i < node->action_len; i++)
			if (regret[i] > 0) {
				if (node->actions[i].actionstr != 'l' && node->actions[i].actionstr != 'd')
					regret[i] *= 10;
			}
	}
	double sum = 0;
	for (int i = 0; i < node->action_len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / node->action_len;
	}
	if (regret[index] > 0)
		return regret[index] / sum;
	else
		return 0;
}
double calculate_avestrategy_action(subgame_node* node, int biasid, int index) {
	assert(node->action_len != 0);
	double regret[12];
	for (int i = 0; i < node->action_len; i++)
		regret[i] = node->actions[i].ave_strategy;
	if (biasid == 1) {
		if (node->actions[0].actionstr == 'd' && regret[0] > 0)
			regret[0] *= 5;
	}
	else if (biasid == 2) {
		if (node->actions[0].actionstr == 'l') {
			if (regret[0] > 0)
				regret[0] *= 5;
		}
		else if (node->actions[0].actionstr == 'l') {
			if (regret[1] > 0)
				regret[1] *= 5;
		}
		else
			throw exception();
	}
	else if (biasid == 3) {
		for (int i = 0; i < node->action_len; i++)
			if (regret[i] > 0) {
				if (node->actions[i].actionstr != 'l' && node->actions[i].actionstr != 'd')
					regret[i] *= 5;
			}
	}
	else if (biasid == 4) {
		for (int i = 0; i < node->action_len; i++)
			if (regret[i] > 0) {
				if (node->actions[i].actionstr != 'l' && node->actions[i].actionstr != 'd')
					regret[i] *= 10;
			}
	}
	double sum = 0;
	for (int i = 0; i < node->action_len; i++) {
		if (regret[i] > 0)
			sum += regret[i];
	}
	if (sum == 0) {//确保后悔值一定有正数
		return 1.0 / node->action_len;
	}
	if (regret[index] > 0)
		return regret[index] / sum;
	else
		return 0;
}
