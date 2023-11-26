#include <vector>
#include <queue>
#include <string>
#include <exception>
using namespace std;

int P = 1;

struct Motif
{
    string text;
    double probability;
    int len;

    Motif(string _text, double _probablity = 1, int _len = -1)
    {
        text = _text;
        probability = _probablity;
        len = _len;
    }
};

class tfem
{ //ProbablisticFindExactMotifs
  private:
    int nMotifs;
    double threshold;

    struct subst
    {
        char c;
        vector<char> list;
        vector<int> index;

        subst(char c_, vector<char> list_)
        {
            c = c_;
            list = list_;
            for (int i = 0; i < list.size(); i++)
                index.push_back(0);
        }
    };

    vector<char> Alphabet{'A', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'Y'};
    vector<subst> sublist{subst('X', vector<char>{'A', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'Y'}), subst('B', vector<char>{'W', 'Y'})};
    vector<Motif> Traverse;

    void initSubs()
    {
        for (int i = 0; i < sublist.size(); i++)
        {
            for (int j = 0; j < sublist[i].list.size(); j++)
            {
                sublist[i].index[j] = getAlphabetIndex(sublist[i].list[j]);
            }
        }
    }

    vector<int> findAllIndexes(string &text, string subtext)
    {
        vector<int> index;
        int start = 0;
        //edit parallel
       // #pragma omp critical
        while ((start = text.find(subtext, start)) != -1)
            index.push_back(start++);
        return index;
    }

    int getAlphabetIndex(char c)
    {
        for (int i = 0; i < Alphabet.size(); i++)
        {
            if (c == Alphabet[i])
                return i;
        }
        return -1;
    }

    vector<int> getAlphabetIndexes(char c)
    {
        int index = getAlphabetIndex(c);
        if (index > -1)
        {
            return vector<int>{index};
        }
        for (int i = 0; i < sublist.size(); i++)
        {
            if (c == sublist[i].c)
            {
                return sublist[i].index;
            }
        }
        cout << "Alphabet not found! " + string(1, c) << endl;
        exit(0);
    }

    void findInitialKeys(vector<string> &seq)
    {
        int cols = Alphabet.size();
        int *keyArray = new int[seq.size() * cols];
        for (int i = 0; i < (seq.size() * cols); i++)
            keyArray[i] = 0;
        for (int i = 0; i < seq.size(); i++)
        {
            for (int j = 0; j < seq[i].size(); j++)
            {
                vector<int> index = getAlphabetIndexes(seq[i][j]);
                for (int k = 0; k < index.size(); k++)
                    keyArray[i * cols + index[k]] = 1;
            }
        }

        
        for (int i = 0; i < Alphabet.size(); i++)
        {
            double Sum = 0;

            for (int j = 0; j < seq.size(); j++)
                Sum += keyArray[j * cols + i];
            double Probablity = (Sum / (double)seq.size());

            if (Probablity >= threshold)
                Traverse.push_back(Motif(string(1, Alphabet[i]), Probablity, 1));
        }
        delete keyArray;
    }

    vector<Motif> ExtendMotif(vector<string> seq, Motif M)
    {
        vector<Motif> ret;
        int cols = Alphabet.size();
        int *leftArray = new int[seq.size() * cols];
        int *rightArray = new int[seq.size() * cols];
        double seqCount = 0;
        #pragma omp parallel for schedule(dynamic,C) num_threads(P) reduction(+:seqCount) 
        for (int i = 0; i < seq.size(); i++)
        {
            for (int k = 0; k < Alphabet.size(); k++)
            {
                leftArray[i * cols + k] = 0;
                rightArray[i * cols + k] = 0;
            }
            vector<int> indexes = findAllIndexes(seq[i], M.text);
            for (int p = 0; p < indexes.size(); p++)
            {
                int j = indexes[p];
                int leftIndex = j - 1;
                int rightIndex = j + M.text.size();
                if (leftIndex >= 0)
                {
                    vector<int> index = getAlphabetIndexes(seq[i][leftIndex]);
                    for (int k = 0; k < index.size(); k++)
                        leftArray[i * cols + index[k]] = 1;
                }
                if (rightIndex < seq[i].size())
                {
                    vector<int> index = getAlphabetIndexes(seq[i][rightIndex]);
                    for (int k = 0; k < index.size(); k++)
                        rightArray[i * cols + index[k]] = 1;
                }
            }
            if (indexes.size() > 0)
                seqCount++;
        }
        if (seqCount > 0)
        {	
        	#pragma omp parallel for num_threads(P)
            for (int i = 0; i < Alphabet.size(); i++)
            {
                double leftSum = 0;
                double rightSum = 0;
                for (int j = 0; j < seq.size(); j++)
                {
                    leftSum += leftArray[j * cols + i];
                    rightSum += rightArray[j * cols + i];
                }
                double leftProbablity = (leftSum / seqCount) * M.probability;
                double rightProbablity = (rightSum / seqCount) * M.probability;
                string leftstr = string(1, Alphabet[i]) + M.text;
                string rightstr = M.text + string(1, Alphabet[i]);
                #pragma omp critical
                {
		            if (leftProbablity >= threshold && leftProbablity > 0)
		                ret.push_back(Motif(string(1, Alphabet[i]) + M.text, leftProbablity, M.len + 1));
		            if (rightProbablity >= threshold && rightProbablity > 0 && leftstr != rightstr)
		                ret.push_back(Motif(M.text + string(1, Alphabet[i]), rightProbablity, M.len + 1));
                }
            }
        }
        delete leftArray;
        delete rightArray;
        return ret;
    }

  public:
    vector<Motif> findMotifs(vector<string> &seq, int _nMotifs, int kWidth, double _threshold)
    {
        nMotifs = _nMotifs;
        threshold = _threshold;
        initSubs();
        findInitialKeys(seq);
        int p = 0; // Traverse pointer  
        //edit parallel  
        //#pragma omp parallel num_threads(14)    
        while (Traverse.size() - p > 0)
        {
            Motif M = Traverse[p];
            if (M.len < kWidth || kWidth == 0)
            {
                vector<Motif> Phi = ExtendMotif(seq, M);
                for (int i = 0; i < Phi.size(); i++)
                {     
                    bool same = false;  
                    //#pragma omp critical(tr)   
                    {           
		                int insertion_point = Traverse.size();
		                for (int j = Traverse.size() - 1; Traverse[j].len == Phi[i].len; j--)
		                {
		                    if (Traverse[j].text == Phi[i].text) 
		                    {
		                        same = true;
		                        Traverse[j].probability = max(Traverse[j].probability, Phi[i].probability);                                                     
		                        break;
		                    }                        
		                    if(Traverse[j].probability > Phi[i].probability)                   
		                        insertion_point--;                           
		                }

		               // if(insertion_point == -1)                    
		               //     Traverse.push_back(Phi[i]);                                           
		                //else 
		                if(!same)                              
		                    Traverse.insert(Traverse.begin() + insertion_point, Phi[i]);
                    
                    }
                }
            }
            if (nMotifs > 0 && p > nMotifs - 1)
            {
            	//#pragma omp critical(tr)   
                Traverse.erase(Traverse.begin());
            }
            else
                p++;
        }
        return Traverse;
    }
};
