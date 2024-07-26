#include "parse_config.h"
#include "process_data.h"
using namespace std;

int main(int argc, char** argv)
{
	if(argc < 2)
		cerr << "please input config file name!" << endl;
	//read config info
	map<string, string> config;
	read_config(argv[1], config);

	//process query data and dict data
	ProcessData pd(config);
	pd.processData(config["query"], false, "\t", 10);
	pd.processData(config["dict"], false);

	//process pinyin
	pd.readPinYin(config["in_pinyin"]);
	pd.readFiltSet(config["filt_set"]);
	pd.filtCHWord();

	//sort chinese and english word and filt less than threshold word
	pd.sortCHENWord();

	//create whole pinyin index, hot chinese word index, hot english word index
	pd.createIndex();
	pd.savePinYin(config["out_pinyin"]);
	pd.savePinYinToWord(config["out_pinyin_word"]);
	pd.saveWordCooccur(config["out_word_cooccur"]);
	return 0;
}
