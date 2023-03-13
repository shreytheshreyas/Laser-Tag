/*
----------------------------------------------------------------------------------
--	(c) Rajesh C Panicker, NUS,
--  Description : AXI Stream Coprocessor (HLS), implementing the sum of 4 numbers
--	License terms :
--	You are free to use this code as long as you
--		(i) DO NOT post a modified version of this on any public repository;
--		(ii) use it only for educational purposes;
--		(iii) accept the responsibility to ensure that your implementation does not violate any intellectual property of any entity.
--		(iv) accept that the program is provided "as is" without warranty of any kind or assurance regarding its suitability for any particular purpose;
--		(v) send an email to rajesh.panicker@ieee.org briefly mentioning its use (except when used for the course EE4218 at the National University of Singapore);
--		(vi) retain this notice in this file or any files derived from this.
----------------------------------------------------------------------------------
*/

//#include "ap_axi_sdata.h" // ap_axis can also be used, but it will include all sideband signals which we don't need
#include "hls_stream.h"
#include "ap_int.h"
#include <math.h>

// Creating a custom structure which includes the data word and TLAST signal.
// ACLK, ARESETN, TREADY, TDATA, TVALID are essential signals for AXIS.
// TLAST is a sideband signal which is optional in AXIS.
// However, it is necessary for us since we connecting M_AXIS to AXI Stream FIFO / AXI DMA.
// So, we create a struct with data (TDATA) and last (TLAST). The rest of the essential AXIS signals are automatically dealt with by the HLS tool.

#define NUMBER_OF_INPUT_WORDS 30  // length of an input vector
#define NUMBER_OF_OUTPUT_WORDS 3  // length of an output vector

#define HIDDEN_LAYER_1_SIZE 25

struct AXIS_wLAST{
	double data;
	bool last;
};

void mlp_solution_hls(hls::stream<AXIS_wLAST>& S_AXIS, hls::stream<AXIS_wLAST>& M_AXIS){
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE axis port=S_AXIS
#pragma HLS INTERFACE axis port=M_AXIS

	// ====================================== weights ==============================================
	double input_weights[NUMBER_OF_INPUT_WORDS][HIDDEN_LAYER_1_SIZE] = {{0.13958499,0.16488439,0.0114955455,0.015478548,0.000505954,0.04594794,0.013517201,0.12162206,0.08970067,0.15353332,-0.0060423785,0.12321514,-0.06828783,-0.10013488,0.071880795,-0.046127763,0.05753978,0.023695217,-0.11853213,-0.10909804,-0.09839983,-0.16490611,-0.12581104,0.057556078,-0.01419702},
			{0.15153801,-0.15509097,-0.12461422,-0.05975962,-0.06793644,-0.078529894,0.088736735,0.097335346,-0.0706028,-0.14321762,-0.057954438,0.030559674,-0.10394761,-0.103784196,-0.028593965,-0.19902751,-0.04948916,-0.16017437,-0.0021327345,-0.17998782,0.09540945,-0.10188264,0.03536354,0.17574859,0.11286144},
			{-0.0427721,0.14095226,0.05629711,0.10316834,-0.012651622,-0.020007357,0.03303135,-0.1567189,-0.02206446,0.14392681,-0.08176492,-0.13886692,-0.16460067,0.10384851,-0.10440825,-0.15690698,-0.09523365,0.06417756,-0.029642805,-0.112066306,-0.17279331,0.0607326,-0.18412647,0.03066267,0.07832509},
			{0.16771472,0.03038384,-0.062875316,0.1359733,-0.123717695,-0.13665497,-0.06982944,-0.117452666,-0.11545686,0.10833279,-0.070525706,-0.14646538,0.008155286,0.10573647,0.076574326,0.041845907,-0.007872938,-0.14657353,0.02069074,0.06961712,-0.07061955,0.15415438,0.103052974,0.17898816,0.14046471},
			{-0.04000266,-0.05928266,0.055943698,-0.14191602,-0.12531804,0.16629928,0.09079066,-0.012626636,-0.010047984,0.2095554,-0.16912258,0.090826154,0.08090466,0.023718314,0.18692838,0.026298212,-0.100198336,-0.107818276,0.18186666,0.15227652,-0.035568878,-0.0668154,0.06149419,0.035721034,-0.051434074},
			{0.036841765,0.1128217,-0.038037777,-0.088165015,-0.1065141,-0.13400042,0.12605602,0.19721164,-0.10377743,-0.06926247,0.17400478,-0.13580622,0.040415138,-0.092819154,0.013373132,0.014910855,-0.08934566,-0.15921566,0.015460803,-0.054309025,-0.15514246,-0.05872456,0.0039890585,0.10544315,0.015017691},
			{-0.08888716,0.028454334,0.15142554,0.16799562,-0.062494673,0.097577006,0.073828146,0.024440166,-0.11196712,-0.12693796,-0.09164376,-0.022479892,0.036103994,-0.16452985,-0.0034647074,-0.05081538,0.09999293,0.16579697,0.116207145,-0.05310695,-0.116096795,-0.18034348,0.0757115,0.14636281,0.07995347},
			{0.10722265,0.14751324,-0.10821207,-0.06282732,-0.1441023,0.064163804,-0.03774443,-0.057233576,0.056009587,0.06489605,0.028306615,0.08758193,-0.13846198,0.00655832,-0.14756443,0.17868792,-0.037411395,-0.16565965,0.15072958,0.09748608,-0.029799506,-0.1100792,-3.8665363e-05,0.15261802,-0.12627433},
			{0.16094697,0.019958958,-0.108886875,-0.075721316,0.15308294,0.05932857,-0.058143258,0.08246323,-0.06013919,0.08518298,-0.04434116,-0.084497534,-0.1705037,0.062471654,-0.08157813,0.016560601,0.16381593,0.13058749,0.13585332,-0.09007679,0.13853082,-0.15566519,0.10508124,-0.102191105,-0.19259329},
			{-0.13394156,-0.057579003,-0.10889345,-0.18381695,-0.03623441,-0.09870803,0.025610251,-0.06632199,0.120480165,-0.14854568,-0.0025558749,-0.019907877,0.003215015,-0.112879455,0.09277445,-0.1279809,0.1915052,-0.0006764272,-0.05257043,-0.08632702,0.17630354,-0.06295654,-0.14121333,0.16785687,-0.083359815},
			{0.15869278,0.049054697,0.16421503,-0.08466855,0.15708622,0.16595107,0.004507502,-0.1136886,-0.04450748,-0.09696029,0.19453292,-0.015892074,0.16647738,-0.012374726,0.057528015,-0.00063332473,0.065190166,-0.02799257,-0.040204085,0.11248934,0.1161651,0.052289106,-0.02093434,0.11059713,0.0868274},
			{0.03417045,-0.04951036,0.06084305,0.06299746,0.05688685,0.04012075,-0.02129226,0.15504412,-0.097406186,-0.096389234,0.064939715,-0.04319401,0.105330944,0.12157415,-0.008017552,0.027525028,0.10625003,0.12091389,0.10215373,-0.15907718,-0.10901942,-0.16197944,0.073385075,-0.08536786,-0.18261345},
			{0.13488743,0.07683772,0.17568213,-0.010872173,-0.15460327,0.023486346,0.14468439,-0.062388808,0.002657561,0.0044847718,-0.17877947,-0.09257098,-0.10630094,0.0070281187,-0.05480945,0.046248585,0.082229406,-0.010123871,-0.03624305,0.02232422,-0.11944389,0.1646906,0.17493518,-0.08712502,-0.16621311},
			{0.024726123,0.16300628,-0.15067413,-0.15207067,0.12634775,-0.16089438,0.13720883,-0.040935524,-0.1063606,-0.101434484,0.09202522,-0.16275363,-0.023699686,0.0145813385,0.094694525,-0.13204081,-0.02219316,0.10733153,0.07272197,0.16128275,0.15933123,0.110714525,0.097417265,-0.15313339,0.09834613},
			{0.08803511,0.105538756,-0.18109097,0.014858622,-0.050234333,0.07664254,0.010606941,-0.16090277,-0.016817192,-0.05846687,0.15563789,-0.14758512,-0.13459069,-0.045554254,-0.1275754,0.049986567,0.031027956,0.13880068,-0.026143009,0.03130901,0.06457968,-0.16421431,-0.14142576,0.045851186,0.09149128},
			{-0.025777832,-0.079816416,-0.14283943,-0.186985,-0.06998544,-0.027389884,-0.1006734,0.09952827,-0.23185605,-0.007959395,0.04020344,-0.097761475,-0.08807806,-0.18894492,0.12836373,-0.08079051,-0.0064397114,-0.15198298,-0.13802493,0.049649946,0.0048505813,-0.09965655,0.14102843,-0.14798537,0.16292253},
			{0.14074385,0.10539362,-0.12281565,-0.13753809,-0.15154944,-0.08364166,0.015369679,0.13299498,0.11494526,-0.15983021,0.10240171,0.17633703,0.03306389,-0.17083648,-0.064585835,-0.13498,0.069460966,0.077313505,0.08625849,-0.10632959,0.024708152,0.060057994,-0.04059965,0.07712385,-0.123242855},
			{0.026985822,0.03266731,0.07394987,-0.07925494,-0.18150806,0.15682146,0.007871473,0.06263086,-0.11155683,0.15953137,0.16014902,-0.08818602,0.09941867,-0.08758386,0.009215064,-0.08457104,0.1731765,0.041403476,0.039640915,-0.0025187416,-0.14673428,-0.03586866,-0.038814764,0.05765651,-0.11989421},
			{-0.085232496,0.09271756,0.0653754,-0.17224635,0.052236423,0.040704697,0.030413838,-0.12067761,0.0776007,0.13699715,-0.00824322,-0.12260367,0.15123883,-0.0900171,-0.042966615,0.15457225,-0.047997195,0.032737777,0.06898605,0.010038716,-0.060958862,-0.020062953,-0.16258492,-0.15862085,0.076197326},
			{0.046537284,-0.11127989,0.15170535,-0.10513504,-0.039882213,-0.10101399,0.053040136,-0.19001132,0.08384932,-0.012427736,0.019074593,0.04425493,-0.16762246,0.04999254,0.0353694,0.16633837,-0.077171445,0.033644065,0.19115204,0.044810396,0.17574921,-0.08461072,-0.028933084,0.049751624,0.17637637},
			{-0.08411861,-0.18073173,-0.09428594,0.2315293,0.07108036,-0.0924079,-0.1381415,-0.16735876,0.12512405,-0.05003512,-0.10644655,0.05031933,0.12207189,-0.05834309,-0.09049567,0.16869082,-0.09066609,0.044649407,0.11808067,0.07093847,-0.04502867,0.13580358,0.14139672,-0.014848858,0.13156752},
			{-0.021411113,-0.07053946,-0.12446298,0.04496278,-0.14983195,-0.008718967,-0.07744824,0.11896979,-0.15624344,-0.058156412,-0.05750902,0.10003418,-0.12876813,-0.07685384,0.13318036,0.1297558,-0.09654569,-0.08082138,0.18193458,0.15864378,-0.009158671,-0.1103338,0.025280675,0.08340314,0.057952486},
			{-0.07415403,-0.14003856,0.096869856,0.06780221,0.13554838,0.10194236,0.14332257,0.07026888,0.11595089,-0.14846012,0.11479941,0.13877836,0.06836805,0.16814026,-0.0035801406,0.16036329,0.019876989,-0.016388949,0.018708587,-0.13935877,-0.15159702,0.143374,-0.104857005,0.10475916,0.091429755},
			{0.1211144,0.14980921,-0.0737973,-0.17597182,-0.13401926,-0.04665388,0.07040448,0.16508344,-0.07658011,-0.19531254,0.11296877,0.10167083,0.15450254,0.07369253,0.126254,-0.0357935,0.12668009,0.1664457,-0.16553311,0.0054722354,-0.1021333,0.019768318,0.06216517,-0.18150522,0.14781539},
			{-0.14411871,0.052587092,0.11080861,-0.10292573,-0.0315257,-0.104172245,0.0035087308,0.087525494,0.07205758,0.080887444,-0.1335077,-0.18102248,0.0025637746,0.124969274,0.17961435,-0.087212466,0.14391257,0.13006905,-0.116809696,-0.09122039,-0.0037280917,0.032511976,-0.06048866,0.16743857,-0.101929605},
			{-0.08416858,0.075624704,-0.043324962,0.010714698,0.038134217,-0.06252123,-0.055910233,0.005377171,0.14969291,-0.14260277,-0.12652716,0.016192257,0.16610762,-0.07721545,-0.14807901,0.13367179,0.03886425,-0.01588521,-0.10892559,-0.14442785,-0.11340768,0.13733426,0.17804483,0.15311438,-0.08715356},
			{-0.051554453,0.05774112,0.10444081,0.03888599,0.094254255,-0.13639659,-0.10820999,-0.01483132,-0.03093612,-0.094268955,-0.14315443,0.11059487,-0.15556261,-0.0878342,-0.11842601,-0.08017692,-0.13729167,-0.16571108,-0.15179968,-0.014619912,-0.022640958,-0.041467156,-0.16036175,0.07264328,0.1474592},
			{-0.10977684,-0.0031760484,-0.1418543,-0.1591651,0.14739403,0.065111876,-0.17148073,-0.15031685,-0.0055985395,0.010373719,-0.19945794,-0.016874507,-0.06973589,0.051711436,0.16862118,-0.11256971,-0.10478807,0.015316172,-0.08237043,-0.16070853,0.07430023,-0.11535574,-0.058844402,-0.16686569,-0.10038039},
			{0.017232405,0.14288408,-0.09213602,-0.08723457,0.16631764,0.14131975,-0.06526151,-0.14737433,-0.04177447,-0.07520153,-0.12183923,-0.107587166,0.10648692,-0.11237794,0.12819044,-0.07027704,-0.041807532,-0.13145806,-0.17838183,0.12741837,-0.17858993,-0.026986483,-0.17193894,-0.06521929,-0.11500446},
			{-0.1803247,-0.12972143,0.055662572,-0.092642516,-0.1447671,-0.1718827,0.07409528,-0.17884156,-0.14803886,-0.043811325,-0.06304538,0.17405525,-0.03977704,-0.1280345,-0.12592225,0.04948241,-0.030217718,0.15131533,-0.15740164,0.02114547,0.05422674,0.14197268,-0.074370936,-0.052911624,0.14466979}};

	double hidden_1_weights[HIDDEN_LAYER_1_SIZE][NUMBER_OF_OUTPUT_WORDS] = {{-0.03657608,-0.040115792,0.010145239},
			{-0.13704967,0.079437956,0.120039806},
			{0.043995544,-0.23154052,-0.085496135},
			{-0.07408395,0.11938119,0.17422041},
			{-0.025324196,-0.06215675,-0.071676284},
			{0.027726367,-0.15213425,0.12082185},
			{-0.08988825,0.09133781,-0.14339902},
			{0.042521823,0.10158476,-0.1701149},
			{-0.06996654,0.20858002,-0.023290558},
			{5.7052384e-05,-0.15276958,-0.31524333},
			{-0.19100603,-0.09448378,0.06762154},
			{0.13220547,-0.0145058045,-0.105433315},
			{0.04770735,0.054252252,-0.01666364},
			{-0.08125844,0.02900853,0.075112745},
			{0.3429321,-0.02296792,-0.2688405},
			{-0.16854924,-0.18247813,0.20897742},
			{0.006099164,0.22994813,-0.27146497},
			{0.12873054,0.09468505,-0.11428749},
			{-0.13683163,-0.1111464,-0.15016712},
			{0.12898828,0.14229144,-0.060495466},
			{-0.15637603,0.044757567,-0.04418306},
			{-0.27676737,0.20354185,0.14086834},
			{-0.04297688,-0.10901103,-0.03746325},
			{-0.10729055,-0.15171981,0.122692816},
			{0.29724813,-0.2243113,0.20290445}};

	// =============================== biases ==================================
	double hidden_1_bias[HIDDEN_LAYER_1_SIZE] = {0.05760619,0.05898425,-0.07738232,0.008683317,0.16709787,-0.10958469,0.0111303255,0.07981217,-0.12030168,0.19257967,-0.110219255,-0.043842643,-0.049399152,-0.12738381,-0.16417632,0.112602964,0.05139141,-0.16383624,-0.106258415,0.1804505,0.06978625,0.07040119,-0.11335878,-0.08003981,-0.012046333};

	double output_bias[NUMBER_OF_OUTPUT_WORDS] = {0.09927703,-0.13175182,-0.15860066};

	int word_cnt;
	//ap_uint<8> sum = 0; // using arbitrary precision
	//int sum = 0;		 // using 32 bit precision
	double inputs[NUMBER_OF_INPUT_WORDS];
	double hidden1[HIDDEN_LAYER_1_SIZE];
	double output[NUMBER_OF_OUTPUT_WORDS];
	double output_softmax[NUMBER_OF_OUTPUT_WORDS];

	AXIS_wLAST read_input, write_output;

		mlp_solution_hls_for1:for(word_cnt = 0; word_cnt < NUMBER_OF_INPUT_WORDS; word_cnt++){
			read_input = S_AXIS.read();
			// read_input is the element (data + other signals) received by our ip through S_AXIS in one clock cycle (which contains one word).
			// read() extracts it from the stream. Overloaded operator >> can also be used.
			//sum += read_input.data; //extracting that word
			inputs[word_cnt] = read_input.data;
			// We are not making using of S_AXIS_TLAST in this example.
			// S_AXIS_TLAST is required only when we are receiving an unknown number of words.
		}

		// ======================== hidden layer 1 =========================================
		mlp_solution_hls_for2:for(int i = 0; i < HIDDEN_LAYER_1_SIZE; i++) {
			#pragma HLS pipeline II=60

			double connection = 0;
			mlp_solution_hls_label1:for(int j = 0; j < NUMBER_OF_INPUT_WORDS; j++) {
				connection += inputs[j] * input_weights[j][i];
			}
			connection += hidden_1_bias[i];
			hidden1[i] = 1.0 / (1.0 + exp(-connection));
		}

		// ============================ output layer ===========================================
		mlp_solution_hls_for3:for(int i = 0; i < NUMBER_OF_OUTPUT_WORDS; i++) {
			#pragma HLS pipeline II=80

			double connection = 0;
			mlp_solution_hls_label2:for(int j = 0; j < HIDDEN_LAYER_1_SIZE; j++) {
				connection += hidden1[j] * hidden_1_weights[j][i];
			}
			connection += output_bias[i];
			output[i] = connection;
		}

		// softmax function
		double m = -INFINITY;
		mlp_solution_hls_for4:for (int i = 0; i < NUMBER_OF_OUTPUT_WORDS; i++){
			if (output[i] > m) {
				m = output[i];
			}
		}

		double sum = 0.0;
		mlp_solution_hls_for5:for (int i = 0; i < NUMBER_OF_OUTPUT_WORDS; i++){
			sum += expf(output[i] - m);
		}

		double offset = m + logf(sum);
		mlp_solution_hls_for6:for (int i = 0; i < NUMBER_OF_OUTPUT_WORDS; i++){
			output_softmax[i] = expf(output[i] - offset);
		}

		// ======================== output =========================================
		mlp_solution_hls_for7:for(word_cnt = 0; word_cnt < NUMBER_OF_OUTPUT_WORDS; word_cnt++){
			//write_output.data = sum.to_int();	// using arbitrary precision
			write_output.data = output_softmax[word_cnt];			// using 32 bit precision
			// write_output is the element sent by our ip through M_AXIS in one clock cycle.
			write_output.last = 0;
			if(word_cnt==NUMBER_OF_OUTPUT_WORDS-1)
			{
				write_output.last = 1;
				// M_AXIS_TLAST is required to be asserted for the last word.
				// Else, the AXI Stream FIFO / AXI DMA will not know if all the words have been received from the co-processor.
			}
			M_AXIS.write(write_output);
			// write() inserts it into the stream. Overloaded operator << can also be used.
		}
}