#include "RankingManager.hpp"

rankingData_S RankingManager::makeRankingData(int time, int miss){
  rankingData_S ret;
  ret.time = time;
  ret.miss = miss;
  ret.score = time + miss * RankingManager::MISS_PENALTY;
  return ret;
}

//  コンストラクタ
RankingManager::RankingManager(void){
  memset(ranking_data, 0, sizeof(ranking_data));
  //  全てのスコアを適当にでかい値にしておく
  for(int i = 0; i < RANKING_DATA_NUM; i++){
    ranking_data[i].score = 3141592;
  }
}

RankingManager::RankingManager(initParam init_param)
  :_initParam(init_param)
{
  memset(ranking_data, 0, sizeof(ranking_data));
  //  全てのスコアを適当にでかい値にしておく
  for(int i = 0; i < RANKING_DATA_NUM; i++){
    ranking_data[i].score = 3141592;
  }
}

void RankingManager::setData(int time_input, int miss_input){
  rankingData_S data_tmp;
  rankingData_S data_new = makeRankingData(time_input, miss_input);
  bool is_replaced = false;
  int new_rank = RANKING_DATA_NUM;
  for(int target = 0; target < RANKING_DATA_NUM; target++){
    //  対象データよりも新しいデータの方がスコアが小さかったら更新する
    if(data_new.score <= ranking_data[target].score){
      if(!is_replaced){
        new_rank = target + 1;
        is_replaced = true;
      }
      data_tmp = ranking_data[target];
      ranking_data[target] = data_new;
      data_new = data_tmp;
    }
  }
  auto itr = _initParam.call_backs.find("onDataAdd");
  if(itr != _initParam.call_backs.end()){
    itr->second(new_rank, ranking_data, RANKING_DATA_NUM);
  }
  return;
}

void RankingManager::clearData(){
  memset(ranking_data, 0, sizeof(ranking_data));
  return;
}

rankingData_S RankingManager::getDataByRank(int rank){
  return ranking_data[rank - 1];
}