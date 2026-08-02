#include "Board.h"

#include <string>
#include <cmath>
#include <bit>

static constexpr uint64_t s_initialValuesP = 18446462598732906495;
static constexpr uint64_t s_initialValuesPs[6] = { 71776119061282560, 4755801206503243842, 2594073385365405732, 9295429630892703873, 576460752303423496, 1152921504606846992 };
static constexpr uint64_t s_initialValuesPW = 65535;
static constexpr uint64_t s_initialValuesPWs[6] = { 65280, 66, 36, 129, 8, 16 };
static constexpr uint64_t s_initialValuesPB = 18446462598732840960;
static constexpr uint64_t s_initialValuesPBs[6] = { 71776119061217280, 4755801206503243776, 2594073385365405696, 9295429630892703744, 576460752303423488, 1152921504606846976 };

static constexpr uint64_t s_initialValueHash = 947986582360952339;

static constexpr std::array<Chess::Piece, 64> s_initialBoardPieces = {
	Chess::ROOK, Chess::KNIGHT, Chess::BISHOP, Chess::QUEEN, Chess::KING, Chess::BISHOP, Chess::KNIGHT, Chess::ROOK,
	Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN,
	Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE,
	Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE,
	Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE,
	Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE, Chess::NONE,
	Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN, Chess::PAWN,
	Chess::ROOK, Chess::KNIGHT, Chess::BISHOP, Chess::QUEEN, Chess::KING, Chess::BISHOP, Chess::KNIGHT, Chess::ROOK
};

static constexpr uint64_t s_rhn_64[] = { 15173973085361057669, 7862768563096822736, 4293682041401933041, 16265580252591650090, 17351771269511602005, 3251009219266959329, 5278960321223168607, 3781993263156988493, 13477741372527711260, 13646870987538715463, 4511861816816422175, 11833663452370212735, 7264961672780572021, 18410250303800398325, 4067145183149845641, 13061324963979732118, 9133052286356221229, 1584072114257690623, 11475377940596413032, 3880814575305468950, 4304752874991332557, 6428104139670995806, 6647187509048560584, 5255227364393674860, 15260086635016315876, 17739719480984730198, 10365652680022999212, 7077838731095659008, 16159542213319848264, 1976403889343757070, 7044911375147417016, 10533554000582968782, 1962138251281139401, 7139800281768225003, 4188424247751692315, 566985385273959839, 11725558239946151017, 5692450993307735290, 7448695322221813676, 703042033890351106, 17649527093385685647, 8532342456362854476, 9362776410515453007, 16455061762763953635, 15818287861857654117, 13802810524400852891, 2032422262554382519, 509662198855079251, 4387930835628978650, 845084269217593916, 522661612063216415, 1966050274594384672, 16762530475975782317, 13957794095038227054, 8628634005148321293, 66646702844091721, 14489448374504255145, 4834176843763231904, 551591847374570676, 11909129645834312418, 444257142538902873, 3217745342705702284, 1911269947332049858, 11534862886305811617, 2804308278224208754, 1711510418124948422, 16108642375585648924, 3235246487401755950, 3409081202953210941, 11464824066322843471, 16144963261259297885, 4370229287899290205, 14383436184429456714, 828991654282637448, 474868497689533689, 5888408157956828723, 14074167005869853548, 7895528547421168950, 11496831256684861893, 8568088638181529049, 7999178371750196342, 8976017223339409582, 8412150601343474480, 11654669294901706677, 17180082437739184730, 18306158651418171, 13488495609006713624, 2994915571085036799, 9169429303141576247, 14293321370492334362, 17237478893060524970, 9117322766797251540, 14322610027969587409, 11882565032842752823, 16884054620220941208, 11974679441119193549, 8405576415974494190, 2431436385490300552, 9070883852500761390, 9970099014513745555, 59383713325054622, 5476683737651889225, 9506728939103787452, 12527874001952196893, 11982780197205634438, 4368712966195553769, 12056562249178972436, 8481062483752148078, 16112106205427606856, 15436873375217849091, 14915272829924084288, 193357835187101103, 18412191855466094597, 4589836694397323799, 8286972916920624635, 15499369962393853632, 11435575078396565177, 11594409268746992322, 14431508377826911607, 17256716197679551114, 13755267372216620920, 17632566019829574719, 5354443562749448420, 15329392163210727304, 212648240996287255, 3919423086202406295, 130947414071792270, 3631494966477214991, 7507552620794491576, 7176846341404380860, 8587159639052572538, 3397572308410854794, 576347695238933629, 17095022050465592939, 16491433893958177143, 9380542700148889434, 7254720938334299467, 6204039395050064930, 140769838100064498, 5581652653608135205, 11534738788925429674, 16914394402015539374, 5742006371732250405, 8921332729631263680, 5544573757315798146, 1592233440158718625, 13904823650151101942, 6250219141020488738, 404407392276133484, 15116828849001452574, 366685681228959621, 2603140918993093470, 1337427765196131254, 6419790085931452743, 13078687984473366279, 2514905041614632804, 9657679935671333702, 640575827420081670, 10077755036961634223, 16547946490988933522, 3767778438394591574, 5569644619194092649, 11536448086401083914, 4470939240773133569, 8851751468228923325, 11909015524612112357, 6318630702649173746, 15724435567743305167, 1304942783841867459, 11378518663787375945, 959028224044696600, 11969056820378429411, 10839428843143671809, 3500095347478872884, 1056736482252654415, 5224215643662752204, 366294069238038948, 9815647466222723811, 8698140111621512453, 16807232653233375125, 488335551017756010, 14103856505720830698, 16743301212676201357, 13798586629000862404, 12809257541262515064, 5579362523220390436, 16138745193377613495, 12030991159820112086, 13937510763883356036, 2990074380575497596, 17303409855714037807, 17715526412962093766, 13041265375870426717, 10031157484404171106, 16457939697064633391, 5767573469211496378, 8865471532100744935, 11630716141327884958, 10474646260704214958, 9805140151889222694, 7277901421575114865, 5825738772903593871, 1875047968357188380, 2776381776285799049, 11466528533848229016, 7816706179763762458, 15878524954979563409, 5374777458124259724, 12485527654347904557, 7446436845467475922, 7681743870915025455, 6245534686090958023, 13974804792059659415, 5775526832221205892, 5560757410728214711, 10789130662378476225, 6935854225079400815, 16738193538227001858, 16051859868196391545, 17430974189644482648, 12078446159529217302, 8448068643478335898, 18111087079327714540, 15081963161324234477, 18235237117921784750, 2549577106117569918, 1588303502547841910, 6587860875340846282, 4407557294638353446, 387425377124848764, 15007268264175005670, 10227253406099468170, 9615940143482302817, 4967406811299436861, 13672002869120702294, 5422627411356689459, 18366224842006937915, 8276594061225517104, 1920862932351068889, 11970044323263700321, 15653757625031189098, 9799858943365170654, 1998338908288066688, 13997854745584334866, 7876405623189449987, 8433316454403626365, 11255143623544922998, 15826837567604663346, 10865932477336790619, 3552836261281522029, 10074504080509231715, 4209337122869166576, 4574362144980943144, 3009440065578376516, 453933402019593236, 9960818084149182052, 11557911032982838496, 6118277848088771271, 15316266056923439867, 17045414554326208565, 15328398955104786516, 755661344885505018, 796429009797173589, 13133752887785971536, 17182573823700111882, 8506749616178021817, 9575872218986439606, 3435966589918669794, 6192702303763085491, 4138764479833704965, 14417343654441426665, 12008225031451222387, 5664605463760398740, 16032955697275368050, 18097429459964542346, 11720549528209329375, 6147450035581035619, 5356456700023596840, 14201782815221212578, 6924974949001141881, 8999390348968872418, 13624743901411557511, 10580623613502104041, 1943455089950290172, 4160225172386431610, 7514973089104003457, 17482381204148468571, 2816100696786284849, 1652399402600844228, 6707843689802421471, 9237729828563728532, 998537347592263847, 15141629553150383215, 4580591792596674247, 17714158056044300788, 10544233279155382155, 4949072133772455906, 14264797823533045383, 8172532556236676127, 6591552780859729783, 277516618693330021, 10204648717502736285, 14503585794399065159, 2938501586418789245, 7632733217269570028, 11521060339206442846, 5313051957226847679, 4505580556467159952, 7996633250643468540, 1722148045537496133, 7007152357327287311, 3818523390283184977, 12912423635017741566, 10670856032242789443, 14861450508519675651, 15322975577342575423, 17635141415336948658, 103788928661125596, 8474604651049254586, 7416125826343503, 18071473728740486803, 3494538786626997145, 3334276875853299511, 8732675166920761060, 5563602116694321858, 7959182444456925911, 7186859766450782550, 12381982691341427848, 17484876659659564134, 9968373552444425377, 18153788742386847984, 43516275854750572, 11613652662084290823, 12418854903638893928, 6336276376744176476, 3596603955219802127, 2577926811381848132, 9791741689532146305, 18261185445552687009, 3446519553104997161, 17194631970207063973, 8203649371815214752, 6379713907777719205, 6199305569189151458, 9007101125566453244, 2566270213973161591, 1788731479562873421, 5789772845843121990, 14513019408576131147, 6388056305756198936, 4464201869945837993, 13858063777669732807, 272242173261302492, 6143619948426759818, 5735104089804815404, 1467043106973563531, 12882478234831852542, 1043211070794250802, 6093095008007917263, 2621633094953047942, 16326198622388410928, 6974143194635894926, 13806962108739892369, 3323503930627517141, 14935467262167820215, 2673712219719895593, 5156054891210555786, 3209860736210733080, 10440134365333701701, 9765593715769632869, 10537286051591322837, 8795603310682850920, 12294437756280463914, 4483737410802864555, 6748320646506267393, 7194264449845708971, 16371646399909224633, 17593201485101780097, 15760175186416277590, 9987904738472659451, 14583035383100955761, 9964683334539326684, 3354687616222569955, 17458999637209804349, 163289004614253065, 9752194815725558710, 9316249056677418964, 1553397584604387544, 4168917774536254430, 13923344336636029166, 13576245189432698984, 11759022172368435789, 14309815916449017210, 500437457793209357, 3202225603564371453, 6596182224467955623, 11402667907361523799, 9310625410434791029, 3848422104297246263, 6003689058289655107, 10495382999724305689, 17229563812374724198, 17761119909396453445, 12991302641138033906, 9493580285207729091, 6922512878368140728, 9221442258319867598, 3920751978323784633, 14987165686451526804, 16654552994894984797, 4552666774176717989, 5075890849823595039, 13147077267889081794, 12895668957902694583, 13608214640595364110, 7410571783394777474, 12110162748158658064, 6726932636941290714, 7900435870525722959, 2023435127758112315, 6338680368119578586, 8896407188460577137, 2698402011665471647, 17228301742503534953, 5537683577816468018, 8587329992329296749, 11484655914138165006, 12179236898521704266, 13133250932197463724, 8998514389884750972, 16326216653812950782, 6354697399266598679, 18106587012890570440, 18094585810030038045, 4291426520188650813, 15383352920508219566, 14990480891220758823, 9081829462994511920, 13683828662872648706, 4753887476922947026, 1912469036921531844, 7112178702525711964, 15870554301847933790, 6102726847578571926, 10820298797825194178, 926965616851204236, 11037227278002950481, 14561150600490615413, 7179123633787161757, 15719423152513120868, 13508379305341314365, 6669911783576799315, 12828089318432474274, 16145285907535810562, 2190795950842910775, 8003971388521785644, 15478023316553331368, 13573435301887829443, 6640857254554642768, 6823522737485854557, 7671789479766987135, 1452512468882449474, 15148512313015502690, 6256226946892322749, 11260501433634484981, 4468094159363794084, 9721148495620021038, 603165757840047117, 13016552067632362885, 1611589250814770615, 2938105447444510893, 17062106414960413098, 16070546695725498313, 3363799338299741216, 11684946696765824804, 14163289592862526581, 909994376009909283, 14250091943468020503, 12293123952293053872, 6560293690948079998, 12986754369819649606, 9568739400597882383, 510951663883606926, 251528669142976791, 3358833542910487363, 11608705863939781088, 13317206011310110715, 10905756309179955307, 14991416360281580848, 8643720462903122190, 573938037753921773, 8013199455081084308, 14335058112998387193, 12063811564925193933, 2618269081563311100, 6537700261328235301, 15993513588507915001, 17668018330257289964, 7618787595745278557, 10433221688388855205, 6635102270633478131, 9447177906591272725, 10316946862024378849, 13987088552509924902, 7006097229249273231, 3687254020795588326, 15716881024499568347, 4670544836861088498, 3757369376932168239, 585228089753638598, 14287597615524685188, 12723547693761256114, 3523989098065341973, 376646644038259094, 9137597602844451267, 655818170081688797, 11987879369405354559, 3204214488168506866, 4356757314252213181, 2410390082719373996, 7590000711536437223, 915347051731571696, 13519399573999354270, 8583428014307675379, 14146427999903695635, 7100228508611778775, 11048603932115992078, 14331176690546900317, 2893971603376211804, 5849970958843136634, 7576151575007603994, 2690605609410846743, 14974869600602063125, 8573455244061032015, 657388770967402158, 17347591317152146894, 811032017274325728, 15160941476192496134, 5542470787171894764, 3802216848666166695, 7562618625291786003, 3908590959098705304, 1160819676288205253, 17887945008564624840, 14726205670080304611, 6773855452434947004, 1764518595030100737, 11407794930614296964, 4279825992257800195, 15764459812813907614, 14212064126895203573, 16210616853183262971, 16925548482473941313, 9161720459417371926, 298419600839422302, 8097640567396486570, 15051989405323496509, 12629415718143572511, 13792522147500004459, 8405767406710440587, 13400512144787259193, 2945902336395816110, 9774796887923955276, 4211372415463788741, 11810813645576255391, 7677879478192328763, 10698781879539544373, 8245889706591578528, 4630376602095381064, 16388181249904046726, 8745809240053733890, 559397680905109910, 10133286342504096472, 3373464441954165812, 16562515305845516333, 17352288506143237582, 17253021385629886428, 17970123505565607831, 11273752995035226972, 15010433659839989136, 12512576193318596313, 6591471810618797005, 7640798190555489668, 11338830817388056402, 8084318921917581534, 14528567323077603096, 11118278874958479878, 4549765373290221102, 9257293810885709783, 11590118985555645407, 10546461469579930459, 18411500434080962609, 2578870988815222997, 8289678023939476771, 5354193480956656585, 8303812590852374542, 4533037383479094692, 17531556353991415443, 7182632263189562240, 1314992548478233515, 16437603271571922862, 6366616528149768287, 15980548232444647061, 7483732100798308082, 16329703057875309232, 13985000058765635748, 7578748605748068527, 7859621985976794103, 4850061119286413987, 10019651395274790708, 15172747901979395413, 9534037520718563335, 14000463495530264576, 4195426538971936160, 2122796824854525729, 2129489710849224206, 143164237263225639, 10550477514198900078, 1790030656423758068, 12072155573458823600, 2308603186842783067, 6071716044590496282, 15779981268846130980, 10233576691277440076, 8919117306470986045, 18143534252480709452, 6818650534577127537, 11355217068401449302, 10309378056659412298, 8241658946108162887, 7604413144296411278, 9004700238466013377, 10605021691467498660, 17472546414326472242, 1000767372620929216, 18257303925174621527, 15896735707750582046, 852889356151457, 3664214816930672358, 72643641077596839, 10849314162434216345, 9173587283974532791, 3585433973980600281, 2905757599327540078, 14494782861659312072, 15411055556614350162, 10866350043098289431, 1975043847482518168, 3142046934870295966, 6782730551396585418, 9846384712022056054, 5337214092323153510, 10173976249920125567, 14635891295744623225, 4945911119481729677, 7247707590828187450, 11296595847292997134, 7081156584790629282, 4358845145502101355, 14402841309369879148, 4970451968392524473, 11032620595115546559, 12294688178383727639, 5567080867623403275, 7358422560424295977, 6370191452201674929, 9479192811660865473, 2746375591505392682, 12713017628377015909, 7545795700107670289, 13456046787765427770, 133850249161621294, 13222403440389565465, 12558633101874265507, 7604736816620463146, 10144821882835909056, 9733205776196857062, 10071299001549309131, 6883389218056695311, 6277613683313189259, 2599502639667820157, 2801189970415680469, 14027453554459607103, 11149171990106861541, 17992614241047304234, 1994850604787826010, 12106396711400978601, 3220357505292875631, 1802198417310964658, 1666038162998533428, 14415733676407508921, 1460426714115776419, 8961598609243403153, 11745340249544303793, 13056106549220738196, 3913025543036902311, 8225362104529533307, 12579165694158680092, 1652529579804291985, 17859185953059053183, 2285210467627617453, 7914296322464907220, 18341078952132222096, 1832017118170566945, 8407698496652685541, 12485470820882487672, 4839314897503643856, 5451145526955620353, 14548735963253522725, 7669743092925301885, 6523689191618032382, 1172564347180577841, 16551173942053806276, 8183281170006330150, 2755470569349888044, 2501276403771422099, 10774638643943139160, 10593472007325034566, 3791511234118735273, 18085448226248937446, 5015276533082208308, 1595149083654954963, 7390130363359790571, 10601273302045465204, 5008418101296283594, 12659312590673540036, 12501077909217630483, 2812890581877817215, 16752142968094889816, 4916399281708929697, 8026929389021525242, 14571409701844688388, 11250501506659089631, 12274512997716141855, 15075810548717537607, 13223282089862451253, 14933403298351871970, 15934216748693425239, 12422072457483768426, 10043717272392299310, 7978952649901705764, 8080906124054274634, 8879636576439132427, 1449393328231994671, 5937371242351962330, 16213441807358619918, 14376821974286603066, 7361590747628947469, 17650277994378903616, 11041162121083307392, 2696700558152574244, 11593820313767623980, 8221600017145144907, 16443492768429158410, 7639773270633355284, 8788501997781006390, 5809778963862481486, 12650064361251150502, 5313826424198196778, 7921293797132030624, 17281742083210876808, 2111372511400362557, 8126142575363945052, 6235350653333863209, 14357163147847960055, 9108158612417543247, 1062689337248456045, 2466006949167109043, 15558492341876652514, 2701327048683012446, 9737818018206299265, 7400159716742157536, 8565375149424644773, 10368554178850712595, 17579106664983173395, 15284676912483711084, 4048892298648994517, 14576345655564020400, 17058887791809034597, 16082298081850151511, 2771968270193915027 };

static constexpr uint64_t s_stages[] = { 0, 64, 64 * 2, 64 * 3, 64 * 4, 64 * 5, 64 * 6, 64 * 7, 64 * 8, 64 * 9, 64 * 10, 64 * 11, 64 * 12, 64 * 12 + 1, 64 * 12 + 2, 64 * 12 + 3, 64 * 12 + 4, 64 * 12 + 5 };

static constexpr std::string_view s_strMoveTypes = " NBRQK";
static constexpr std::string_view s_strMoveTypesSmall = " nbrqk";
static constexpr std::string_view s_strMoveIndexX = "abcdefgh";
static constexpr std::string_view s_strMoveIndexY = "12345678";

namespace Chess
{
	Board::Board()
	{
		NewPosition();
	}

	std::string Board::GetFen() const
	{
		std::string Fenstr = "";

		std::string piecenames = "pnbrqkPNBRQK";

		for (int y = 7; y > -1; y--)
		{
			char empty = '0';
			for (int x = 0; x < 8; x++)
			{
				if (m_BoardPieces[x + 8 * y] != NONE)
				{
					if (empty != '0') { Fenstr += empty; empty = '0'; }
					Fenstr += piecenames[m_BoardPieces[x + 8 * y] + 6 * m_WhitePieces.At(x + 8 * y)];
				}
				else
					empty += 1;
			}
			if (empty != '0') { Fenstr += empty; }
			if (y != 0) { Fenstr += '/'; }
		}

		Fenstr += ' ';

		if (m_PlayerToPlay)
			Fenstr += 'w';
		else
			Fenstr += 'b';

		Fenstr += ' ';

		bool roke = false;

		if (m_mapWhitePieces[KING].At(4))
		{
			if (m_mapWhitePieces[ROOK].At(7) && m_K)
			{
				Fenstr += 'K';
				roke = true;
			}

			if (m_mapWhitePieces[ROOK].At(0) && m_Q)
			{
				Fenstr += 'Q';
				roke = true;
			}
		}

		if (m_mapBlackPieces[KING].At(60))
		{
			if (m_mapBlackPieces[ROOK].At(63) && m_k)
			{
				Fenstr += 'k';
				roke = true;
			}

			if (m_mapBlackPieces[ROOK].At(56) && m_q)
			{
				Fenstr += 'q';
				roke = true;
			}
		}

		if (!roke) { Fenstr += '-'; }

		Fenstr += ' ';

		if (m_LastMovedPieceIndex == -1)
		{
			Fenstr += '-';
		}
		else
		{
			Fenstr += 'a' + m_LastMovedPieceIndex % 8;
			
			if ((m_LastMovedPieceIndex / 8.0f) > 4)
				Fenstr += '6';
			else
				Fenstr += '3';
		}

		Fenstr += (' ' + std::to_string(m_FiftyMoveCounter));
		Fenstr += (' ' + std::to_string(m_BlackMovesCounter));

		return Fenstr;
	}

	std::vector<uint8_t>  Board::GetFormatedFen() const
	{
		BitBoard allPieces = m_WhitePieces + m_BlackPieces;

		std::vector<uint8_t> pos;
		int blockFilled = 0;
		bool firstfour = true;
		int blocks_empty = 0;

		constexpr uint8_t classnames[6] = { 0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000100, 0b00000101 };
		constexpr uint8_t color[2] = { 0b00000000, 0b00001000 };
		constexpr uint8_t empty[4] = { 0b00000110, 0b00001110, 0b00000111, 0b00001111 };

		pos.emplace_back(0b00000000);
		for (int y = 7; y >= 0; y--)
		{
			for (int x = 0; x < 8; x++)
			{
				if (blockFilled == 8)
				{
					blockFilled = 0;
					pos.emplace_back(0b00000000);
				}
				if (!allPieces.At(x, y))
				{
					blocks_empty += 1;
					if (blocks_empty == 4)
					{
						size_t s = pos.size();
						pos[s - 1] |= empty[blocks_empty - 1];
						if (firstfour)
						{
							pos[s - 1] <<= 4;
							firstfour = false;
						}
						else
							firstfour = true;
						blockFilled += 4;
						blocks_empty = 0;
					}
					continue;
				}
				else if (blocks_empty)
				{
					size_t s = pos.size();
					pos[s - 1] |= empty[blocks_empty - 1];
					if (firstfour)
					{
						pos[s - 1] <<= 4;
						firstfour = false;
					}
					else
						firstfour = true;
					blockFilled += 4;
					blocks_empty = 0;
					if (blockFilled == 8)
					{
						blockFilled = 0;
						pos.emplace_back(0b00000000);
					}
				}
				size_t s = pos.size();
				pos[s - 1] |= classnames[m_BoardPieces[x + 8*y]];
				pos[s - 1] |= color[(m_WhitePieces.At(x + 8 * y) ? 1 : 0)];
				if (firstfour)
				{
					pos[s - 1] <<= 4;
					firstfour = false;
				}
				else
					firstfour = true;
				blockFilled += 4;
			}

		}
		if (blocks_empty)
		{
			size_t s = pos.size();
			pos[s - 1] |= empty[blocks_empty - 1];
			if (firstfour)
				pos[s - 1] <<= 4;
		}
		
		pos.emplace_back(((uint8_t)((m_LastMovedPieceIndex == -1 ? 0 : m_LastMovedPieceIndex) % 8) | (m_PlayerToPlay == WHITE ? 0b00001000 : 0b00000000)) << 4);
		
		size_t s = pos.size();
		
		if (m_mapWhitePieces[KING].At(4))
		{
			if (m_mapWhitePieces[ROOK].At(7) && m_K)
				pos[s - 1] |= 0b00000001;

			if (m_mapWhitePieces[ROOK].At(0) && m_Q)
				pos[s - 1] |= 0b00000010;
		}

		if (m_mapBlackPieces[KING].At(60))
		{
			if (m_mapBlackPieces[ROOK].At(63) && m_k)
				pos[s - 1] |= 0b00000100;

			if (m_mapBlackPieces[ROOK].At(56) && m_q)
				pos[s - 1] |= 0b00001000;
		}

		return pos;
	}

	std::array<uint8_t, 149> Board::GetBinFixedFen() const
	{
		std::array<uint8_t, 149> pos;
		pos.fill(0);
		
		*(uint64_t*)&pos[0] = m_WhitePieces.Data();
		*(uint64_t*)&pos[8] = m_BlackPieces.Data();
		*(uint64_t*)&pos[16] = m_mapWhitePieces[KING].Data();
		*(uint64_t*)&pos[24] = m_mapBlackPieces[KING].Data();
		*(uint64_t*)&pos[32] = m_mapWhitePieces[QUEEN].Data();
		*(uint64_t*)&pos[40] = m_mapBlackPieces[QUEEN].Data();
		*(uint64_t*)&pos[48] = m_mapWhitePieces[ROOK].Data();
		*(uint64_t*)&pos[56] = m_mapBlackPieces[ROOK].Data();
		*(uint64_t*)&pos[64] = m_mapWhitePieces[BISHOP].Data();
		*(uint64_t*)&pos[72] = m_mapBlackPieces[BISHOP].Data();
		*(uint64_t*)&pos[80] = m_mapWhitePieces[KNIGHT].Data();
		*(uint64_t*)&pos[88] = m_mapBlackPieces[KNIGHT].Data();
		*(uint64_t*)&pos[96] = m_mapWhitePieces[PAWN].Data();
		*(uint64_t*)&pos[104] = m_mapBlackPieces[PAWN].Data();

		if (m_K)
			pos[112] |= 0b00000001;
		if (m_Q)
			pos[112] |= 0b00000010;
		if (m_k)
			pos[112] |= 0b00000100;
		if (m_q)
			pos[112] |= 0b00001000;
		if (m_PlayerToPlay == WHITE)
			pos[112] |= 0b00010000;

		pos[113] = m_LastMovedPieceIndex == -1 ? 8 : m_LastMovedPieceIndex;
		pos[114] = m_FiftyMoveCounter;
		*(uint16_t*)&pos[115] = m_BlackMovesCounter;

		for (int i = 0; i < 32; i++)
			pos[117 + i] = m_BoardPieces[i * 2] << 4 | m_BoardPieces[i * 2 + 1];
	
		return pos;
	}

	uint64_t Board::GetHash(bool recalculate) const
	{
		if (!recalculate && m_Hash != 0)
			return m_Hash;

		uint64_t hash = 0;
		
		for (int i = 0; i < 64; i++)
		{
			int pieceType = GetPieceType(i);
			int pieceColor = GetPieceColor(i);

			if (pieceType != NONE)
			{
				hash ^= s_rhn_64[s_stages[pieceType + 6 * pieceColor] + i];
			}
		}

		if (m_PlayerToPlay == BLACK)
			hash ^= s_rhn_64[s_stages[12]];

		if (m_K) { hash ^= s_rhn_64[s_stages[13]]; }
		if (m_Q) { hash ^= s_rhn_64[s_stages[14]]; }
		if (m_k) { hash ^= s_rhn_64[s_stages[15]]; }
		if (m_q) { hash ^= s_rhn_64[s_stages[16]]; }

		if (m_LastMovedPieceIndex != -1)
			hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];
		
		return hash;
	}

	bool Board::NewPosition(const std::string& fenStr)
	{
		if (fenStr == "?" || fenStr == "" || fenStr == "default" || fenStr == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
		{
			//m_Pieces.Data() = s_initialValuesP;
			m_WhitePieces.Data() = s_initialValuesPW;
			m_BlackPieces.Data() = s_initialValuesPB;

			//for (int i = 0; i < 6; i++)
			//	m_mapPieces[i].Data() = s_initialValuesPs[i];
			for (int i = 0; i < 6; i++)
				m_mapWhitePieces[i].Data() = s_initialValuesPWs[i];
			for (int i = 0; i < 6; i++)
				m_mapBlackPieces[i].Data() = s_initialValuesPBs[i];

			m_K = true;
			m_Q = true;
			m_k = true;
			m_q = true;

			m_LastMovedPieceIndex = -1;

			m_PlayerToPlay = WHITE;

			m_FiftyMoveCounter = 0;
			m_BlackMovesCounter = 1;

			m_Hash = s_initialValueHash;

			m_BoardPieces = s_initialBoardPieces;

			return true;
		}

		std::string fen = fenStr;

		//m_Pieces.Data() = 0x00;
		m_WhitePieces.Data() = 0x00;
		m_BlackPieces.Data() = 0x00;
		//for(auto& piece : m_mapPieces)
		//	piece.Data() = 0x00;
		for(auto& piece : m_mapWhitePieces)
			piece.Data() = 0x00;
		for(auto& piece : m_mapBlackPieces)
			piece.Data() = 0x00;

		m_K = false;
		m_Q = false;
		m_k = false;
		m_q = false;

		m_LastMovedPieceIndex = -1;

		m_PlayerToPlay = WHITE;

		m_FiftyMoveCounter = 0;
		m_BlackMovesCounter = 1;

		m_BoardPieces.fill(NONE);

		//Reading FEN 

		static const std::string WhiteTypePiecies = "PNBRQK";
		static const std::string BlackTypePiecies = "pnbrqk";
		std::string Roke;
		std::string FiftyMoveRule = "";
		std::string Blackmove = "";

		int ylevel = 7, xlevel = 0;

		for (size_t i = 0; i < fen.size(); i++)
		{

			if (fen[i] == '/' || fen[i] == ' ') { ylevel -= 1; xlevel = 0; }
			else if (ylevel >= 0)
			{
				if (fen[i] > 'A')
				{
					//m_Pieces.Set(xlevel, ylevel, true);
					if (fen[i] > 'a') 
					{
						m_BlackPieces.Set(xlevel, ylevel, true);
						//m_mapPieces[BlackTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
						m_mapBlackPieces[BlackTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);

						m_BoardPieces[xlevel + 8 * ylevel] = (Piece)(BlackTypePiecies.find(fen[i]));
					}
					else
					{
						m_WhitePieces.Set(xlevel, ylevel, true);
						//m_mapPieces[WhiteTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
						m_mapWhitePieces[WhiteTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);

						m_BoardPieces[xlevel + 8 * ylevel] = (Piece)(WhiteTypePiecies.find(fen[i]));
					}

					xlevel += 1;
				}
				else
				{
					xlevel += (int)fen[i] - 48;
				}
			}
			else if (ylevel == -1)
			{
				if (fen[i] == 'w') { m_PlayerToPlay = WHITE; }
				else { m_PlayerToPlay = BLACK; }
			}
			else if (ylevel == -2)
			{
				if (fen[i] == '-')
					continue;
				else
					Roke += fen[i];
			}
			else if (ylevel == -3)
			{
				if (fen[i] == '-')
					continue;
				
				if (fen[i] >= 'a')
					m_LastMovedPieceIndex = (int)(fen[i] - 'a');
				else if (fen[i] == '6')
					m_LastMovedPieceIndex += (5 * 8);
				else
					m_LastMovedPieceIndex += (2 * 8);
			}
			else if (ylevel == -4)
			{
				FiftyMoveRule += fen[i];
			}
			else
			{
				Blackmove += fen[i];
			}

		}

		if (Roke.find('K') != std::string::npos)
			m_K = true;
		if (Roke.find('Q') != std::string::npos)
			m_Q = true;
		if (Roke.find('k') != std::string::npos)
			m_k = true;
		if (Roke.find('q') != std::string::npos)
			m_q = true;

		m_FiftyMoveCounter = std::stoi(FiftyMoveRule);
		m_BlackMovesCounter = std::stoi(Blackmove);

		bool ret = IsBoardValid();

		if (!ret)
			NewPosition();

		m_Hash = GetHash(true);

		return ret;
	}

	bool Board::NewPosition(const std::vector<uint8_t>& ffen)
	{
		return false;
	}

	bool Board::NewPosition(const std::array<uint8_t, 149>& bffen, uint64_t fenHash)
	{
		m_WhitePieces.Data() = *(uint64_t*)&bffen[0];
		m_BlackPieces.Data() = *(uint64_t*)&bffen[8];
		m_mapWhitePieces[KING].Data() = *(uint64_t*)&bffen[16];
		m_mapBlackPieces[KING].Data() = *(uint64_t*)&bffen[24];
		m_mapWhitePieces[QUEEN].Data() = *(uint64_t*)&bffen[32];
		m_mapBlackPieces[QUEEN].Data() = *(uint64_t*)&bffen[40];
		m_mapWhitePieces[ROOK].Data() = *(uint64_t*)&bffen[48];
		m_mapBlackPieces[ROOK].Data() = *(uint64_t*)&bffen[56];
		m_mapWhitePieces[BISHOP].Data() = *(uint64_t*)&bffen[64];
		m_mapBlackPieces[BISHOP].Data() = *(uint64_t*)&bffen[72];
		m_mapWhitePieces[KNIGHT].Data() = *(uint64_t*)&bffen[80];
		m_mapBlackPieces[KNIGHT].Data() = *(uint64_t*)&bffen[88];
		m_mapWhitePieces[PAWN].Data() = *(uint64_t*)&bffen[96];
		m_mapBlackPieces[PAWN].Data() = *(uint64_t*)&bffen[104];
		
		m_K = (bffen[112] & 0b00000001) != 0;
		m_Q = (bffen[112] & 0b00000010) != 0;
		m_k = (bffen[112] & 0b00000100) != 0;
		m_q = (bffen[112] & 0b00001000) != 0;
		m_PlayerToPlay = (bffen[112] & 0b00010000) != 0 ? WHITE : BLACK;

		m_LastMovedPieceIndex = bffen[113] == 8 ? -1 : bffen[113];

		m_FiftyMoveCounter = bffen[114];

		m_BlackMovesCounter = *(uint16_t*)&bffen[115];

		for (int i = 0; i < 32; i++)
		{
			m_BoardPieces[i * 2] = (Piece)(bffen[117 + i] >> 4);
			m_BoardPieces[i * 2 + 1] = (Piece)(bffen[117 + i] & 0b00001111);
		}

		m_Hash = fenHash != 0 ? fenHash : GetHash(true);

		return true;
	}

	void Board::GetAvailableMoves(std::vector<Move>& moves) const
	{
		moves.clear();

		FindPawnMoves(moves);
		FindKngihtMoves(moves);
		FindBishopMoves(moves);
		FindRookMoves(moves);
		FindQueenMoves(moves);
		FindKingMoves(moves);

		for (int i = 0; i < moves.size(); i++)
		{
			if (!IsMoveKingSecured(moves[i]))
			{
				moves.erase(moves.begin() + i);
				i--;
			}
		}
	}

	void Board::GetAvailableMoves(std::vector<Move>& moves, Piece type) const
	{
		moves.clear();

		switch (type)
		{
		case Chess::PAWN:
			FindPawnMoves(moves);
			break;
		case Chess::KNIGHT:
			FindKngihtMoves(moves);
			break;
		case Chess::BISHOP:
			FindBishopMoves(moves);
			break;
		case Chess::ROOK:
			FindRookMoves(moves);
			break;
		case Chess::QUEEN:
			FindQueenMoves(moves);
			break;
		case Chess::KING:
			FindKingMoves(moves);
			break;
		default:
			break;
		}

		for (int i = 0; i < moves.size(); i++)
		{
			if (!IsMoveKingSecured(moves[i]))
			{
				moves.erase(moves.begin() + i);
				i--;
			}
		}
	}

	int Board::GetAmountOfPieces() const
	{
		return std::popcount(m_WhitePieces.Data() | m_BlackPieces.Data());
	}

	int Board::GetAmountOfPieces(Piece type) const
	{
		return std::popcount(m_mapWhitePieces[type].Data() | m_mapBlackPieces[type].Data());
	}

	BitBoard Board::GetBitBoard(Piece type, Color color) const
	{
		if (color == WHITE)
			return m_mapWhitePieces[type];
		else
			return m_mapBlackPieces[type];
	}

	Piece Board::GetPieceType(int index) const
	{
		return m_BoardPieces[index];
	}

	Piece Board::GetPieceType(int indexX, int indexY) const
	{
		return m_BoardPieces[indexX + 8 * indexY];
	}

	Color Board::GetPieceColor(int index) const
	{
		if (m_WhitePieces.At(index))
			return WHITE;
		else
			return BLACK;
	}

	Color Board::GetPieceColor(int indexX, int indexY) const
	{
		if (m_WhitePieces.At(indexX + 8 * indexY))
			return WHITE;
		else
			return BLACK;
	}

	Board::KingStatus Board::GetKingStatus() const
	{
		if (m_PlayerToPlay == WHITE)
		{
			int kingIndex = std::countr_zero(m_mapWhitePieces[KING].Data());
			
			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			const int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			BitBoard enemyQueenBishop = m_mapBlackPieces[QUEEN] + m_mapBlackPieces[BISHOP];
			BitBoard enemyQueenRook = m_mapBlackPieces[QUEEN] + m_mapBlackPieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y))
					break;
			}

			if ((m_mapBlackPieces[PAWN].At(kingIndex + 7) && left) || (m_mapBlackPieces[PAWN].At(kingIndex + 9) && right))
				goto skip;

			if (up >= 2)
			{
				if (left && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					goto skip;
				if (right && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					goto skip;
			}
			if (down >= 2)
			{
				if (left && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					goto skip;
				if (right && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					goto skip;
			}
			if (left >= 2)
			{
				if (down && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					goto skip;
				if (up && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					goto skip;
			}
			if (right >= 2)
			{
				if (down && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					goto skip;
				if (up && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					goto skip;
			}
		}
		else
		{
			int kingIndex = std::countr_zero(m_mapBlackPieces[KING].Data());

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			BitBoard enemyQueenBishop = m_mapWhitePieces[QUEEN] + m_mapWhitePieces[BISHOP];
			BitBoard enemyQueenRook = m_mapWhitePieces[QUEEN] + m_mapWhitePieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y))
					break;
			}

			if ((m_mapWhitePieces[PAWN].At(kingIndex - 9) && left) || (m_mapWhitePieces[PAWN].At(kingIndex - 7) && right))
				goto skip;

			if (up >= 2)
			{
				if (left && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					goto skip;
				if (right && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					goto skip;
			}
			if (down >= 2)
			{
				if (left && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					goto skip;
				if (right && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					goto skip;
			}
			if (left >= 2)
			{
				if (down && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					goto skip;
				if (up && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					goto skip;
			}
			if (right >= 2)
			{
				if (down && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					goto skip;
				if (up && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					goto skip;
			}
		}

		return SECURE;

		skip:

		std::vector<Move> moves;
		GetAvailableMoves(moves);

		if (moves.empty())
			return MATED;
		
		return CHECKED;
	}

	Board::MakeMoveStatus Board::MakeMove(Move move, Piece piecePromotion)
	{
		//safe
		//if (!IsMoveValid(move))
		//	return MOVEERROR;

		Color nextToPlay = m_PlayerToPlay;
		Piece type = NONE, enemyType = NONE;
		int direction = move.index + move.move;

		if (move.index > 63 || move.index < 0 || direction > 63 || direction < 0)
			return MOVEERROR;

		type = m_BoardPieces[move.index];
		enemyType = m_BoardPieces[direction];

		//safe
		if (type == NONE)
			return MOVEERROR;

		if (type > 6)
		{
			//__debugbreak();
			return MOVEERROR;
		}

		//safe
		if (type == PAWN && ((direction / 8) == 0 || (direction / 8) == 7) && (piecePromotion > QUEEN || piecePromotion == PAWN))
			return PROMOTION;

		//Roke
		if (type == KING && std::abs(move.move) == 2)
		{
			if (MakeMove(Move((move.move > 0 ? (move.index + 3ui8) : (move.index - 4ui8)), (move.move > 0 ? -2 : 3))) != SUCCESS)
				return MOVEERROR;//safe

			m_PlayerToPlay = nextToPlay;
			m_Hash ^= s_rhn_64[s_stages[12]];

			if (m_PlayerToPlay == BLACK)
				m_BlackMovesCounter--;
			m_FiftyMoveCounter--;
		}

		m_BoardPieces[move.index] = NONE;
		m_BoardPieces[direction] = type;

		if (m_PlayerToPlay == WHITE)
		{
			nextToPlay = BLACK;

			//Moves king or rook so no roke any more
			if (type == KING)
			{
				if (m_K)
					m_Hash ^= s_rhn_64[s_stages[13]];
				m_K = false;

				if (m_Q)
					m_Hash ^= s_rhn_64[s_stages[14]];
				m_Q = false;
			}
			if (type == ROOK && move.index == 7)
			{
				if (m_K)
					m_Hash ^= s_rhn_64[s_stages[13]];
				m_K = false;

			}
			if (type == ROOK && move.index == 0)
			{
				if (m_Q)
					m_Hash ^= s_rhn_64[s_stages[14]];
				m_Q = false;
			}

			m_WhitePieces.FlipMulty(move.index, direction);
			m_mapWhitePieces[type].FlipMulty(move.index, direction);

			m_Hash ^= s_rhn_64[s_stages[type + 6 * WHITE] + move.index] ^ s_rhn_64[s_stages[type + 6 * WHITE] + direction];
			
			if (enemyType != NONE)
			{
				BitBoard::ApplyMaskMany_Set(m_BlackPieces, m_mapBlackPieces[enemyType], direction, false);
				m_Hash ^= s_rhn_64[s_stages[enemyType + 6 * BLACK] + direction];
			}
			
			//White AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				BitBoard::ApplyMaskMany_Set(m_BlackPieces, m_mapBlackPieces[PAWN], direction - 8, false);
				m_Hash ^= s_rhn_64[s_stages[PAWN + 6 * BLACK] + direction - 8];

				m_BoardPieces[direction - 8] = NONE;
			}

			//Promosion
			if (piecePromotion < KING && piecePromotion > PAWN && type == PAWN && direction / 8 == 7)
			{
				BitBoard::ApplyMaskMany_Flip(m_mapWhitePieces[PAWN], m_mapWhitePieces[piecePromotion], direction);
				m_Hash ^= s_rhn_64[s_stages[PAWN + 6 * WHITE] + direction] ^ s_rhn_64[s_stages[piecePromotion + 6 * WHITE] + direction];
				
				m_BoardPieces[direction] = piecePromotion;
			}
		}
		else
		{
			nextToPlay = WHITE;

			//Moves king or rook so no roke any more
			if (type == KING)
			{
				if (m_k)
					m_Hash ^= s_rhn_64[s_stages[15]];
				m_k = false;

				if (m_q)
					m_Hash ^= s_rhn_64[s_stages[16]];
				m_q = false;
			}
			if (type == ROOK && move.index == 63)
			{
				if (m_k)
					m_Hash ^= s_rhn_64[s_stages[15]];
				m_k = false;
			}
			if (type == ROOK && move.index == 56)
			{
				if (m_q)
					m_Hash ^= s_rhn_64[s_stages[16]];
				m_q = false;
			}

			m_BlackPieces.FlipMulty(move.index, direction);
			m_mapBlackPieces[type].FlipMulty(move.index, direction);

			m_Hash ^= s_rhn_64[s_stages[type + 6 * BLACK] + move.index] ^ s_rhn_64[s_stages[type + 6 * BLACK] + direction];
			
			if (enemyType != NONE)
			{
				BitBoard::ApplyMaskMany_Set(m_WhitePieces, m_mapWhitePieces[enemyType], direction, false);
				m_Hash ^= s_rhn_64[s_stages[enemyType + 6 * WHITE] + direction];
			}

			//Black AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				BitBoard::ApplyMaskMany_Set(m_WhitePieces, m_mapWhitePieces[PAWN], direction + 8, false);
				m_Hash ^= s_rhn_64[s_stages[PAWN + 6 * WHITE] + direction + 8];

				m_BoardPieces[direction + 8] = NONE;
			}

			//Promosion
			if (piecePromotion < KING && piecePromotion > PAWN && type == PAWN && direction / 8 == 0)
			{
				BitBoard::ApplyMaskMany_Flip(m_mapBlackPieces[PAWN], m_mapBlackPieces[piecePromotion], direction);
				m_Hash ^= s_rhn_64[s_stages[PAWN + 6 * BLACK] + direction] ^ s_rhn_64[s_stages[piecePromotion + 6 * BLACK] + direction];

				m_BoardPieces[direction] = piecePromotion;
			}
			
			m_BlackMovesCounter++;
		}

		//Update last move for an pan san
		if (type == PAWN && std::abs(move.move) == 16)
		{
			if (m_LastMovedPieceIndex != -1)
				m_Hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];

			m_LastMovedPieceIndex = direction - move.move / 2;
			m_Hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];
		}
		else
		{
			if (m_LastMovedPieceIndex != -1)
				m_Hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];

			m_LastMovedPieceIndex = -1;
		}

		m_FiftyMoveCounter++;

		if (type == PAWN || enemyType != NONE)
			m_FiftyMoveCounter = 0;

		m_PlayerToPlay = nextToPlay;
		m_Hash ^= s_rhn_64[s_stages[12]];

		return SUCCESS;
	}

	bool Board::IsMoveValid(Move move) const
	{
		std::vector<Move> PossibleMoves;
		GetAvailableMoves(PossibleMoves);
		for (auto& m : PossibleMoves)
		{
			if (m.index == move.index && m.move == move.move)
				return true;
		}

		return false;
	}

	void Board::AddPiece(Piece type, Color color, int index)
	{
		Piece old = GetPieceType(index);
		if (old != NONE)
		{
			//m_mapPieces[old].Set(index, false);
			m_WhitePieces.Set(index, false);
			m_mapWhitePieces[old].Set(index, false);
			m_BlackPieces.Set(index, false);
			m_mapBlackPieces[old].Set(index, false);
			m_Hash ^= s_rhn_64[s_stages[old + 6 * (GetPieceColor(index) == WHITE ? WHITE : BLACK)] + index];
		}

		//m_Pieces.Set(index, true);
		//m_mapPieces[type].Set(index, true);
		m_Hash ^= s_rhn_64[s_stages[type + 6 * color] + index];
		
		m_BoardPieces[index] = type;

		if (color == WHITE)
		{
			m_WhitePieces.Set(index, true);
			m_mapWhitePieces[type].Set(index, true);
		}
		else
		{
			m_BlackPieces.Set(index, true);
			m_mapBlackPieces[type].Set(index, true);
		}
	}

	void Board::AddPiece(Piece type, Color color, int indexX, int indexY)
	{
		Piece old = GetPieceType(indexX, indexY);
		if (old != NONE)
		{
			//m_mapPieces[old].Set(indexX, indexY, false);
			m_WhitePieces.Set(indexX, indexY, false);
			m_mapWhitePieces[old].Set(indexX, indexY, false);
			m_BlackPieces.Set(indexX, indexY, false);
			m_mapBlackPieces[old].Set(indexX, indexY, false);
			m_Hash ^= s_rhn_64[s_stages[old + 6 * (GetPieceColor(indexX, indexY) == WHITE ? WHITE : BLACK)] + (indexX + indexY * 8)];
		}

		//m_Pieces.Set(indexX, indexY, true);
		//m_mapPieces[type].Set(indexX, indexY, true);
		m_Hash ^= s_rhn_64[s_stages[type + 6 * color] + (indexX + indexY * 8)];

		m_BoardPieces[indexX + 8 * indexY] = type;

		if (color == WHITE)
		{
			m_WhitePieces.Set(indexX, indexY, true);
			m_mapWhitePieces[type].Set(indexX, indexY, true);
		}
		else
		{
			m_BlackPieces.Set(indexX, indexY, true);
			m_mapBlackPieces[type].Set(indexX, indexY, true);
		}
	}

	void Board::RemovePiece(int index)
	{
		Piece type = GetPieceType(index);

		if (type == NONE)
			return;

		//m_Pieces.Set(index, false);
		//m_mapPieces[type].Set(index, false);

		m_BoardPieces[index] = NONE;

		if (GetPieceColor(index) == WHITE)
		{
			m_WhitePieces.Set(index, false);
			m_mapWhitePieces[type].Set(index, false);
			m_Hash ^= s_rhn_64[s_stages[type + 6 * WHITE] + index];
		}
		else
		{
			m_BlackPieces.Set(index, false);
			m_mapBlackPieces[type].Set(index, false);
			m_Hash ^= s_rhn_64[s_stages[type + 6 * BLACK] + index];
		}
	}

	void Board::RemovePiece(int indexX, int indexY)
	{
		Piece type = GetPieceType(indexX, indexY);

		if (type == NONE)
			return;

		//m_Pieces.Set(indexX, indexY, false);
		//m_mapPieces[type].Set(indexX, indexY, false);

		m_BoardPieces[indexX + 8 * indexY] = NONE;

		if (GetPieceColor(indexX, indexY) == WHITE)
		{
			m_WhitePieces.Set(indexX, indexY, false);
			m_mapWhitePieces[type].Set(indexX, indexY, false);
			m_Hash ^= s_rhn_64[s_stages[type + 6 * WHITE] + (indexX + indexY * 8)];
		}
		else
		{
			m_BlackPieces.Set(indexX, indexY, false);
			m_mapBlackPieces[type].Set(indexX, indexY, false);
			m_Hash ^= s_rhn_64[s_stages[type + 6 * BLACK] + (indexX + indexY * 8)];
		}
	}

	void Board::UpdateVitualValues() const
	{
		m_Virtual_Pieces = m_WhitePieces + m_BlackPieces;
		m_Virtual_WhitePieces = m_WhitePieces;
		m_Virtual_BlackPieces = m_BlackPieces;

		for (int i = 0; i < 6; i++)
			m_Virtual_mapPieces[i] = m_mapWhitePieces[i] + m_mapBlackPieces[i];

		m_Virtual_mapWhitePieces = m_mapWhitePieces;
		m_Virtual_mapBlackPieces = m_mapBlackPieces;
	}

	Board::MakeMoveStatus Board::VirtualMakeMove(Move move) const
	{
		Piece type = NONE, enemyType = NONE;
		int direction = move.index + move.move;

		for (int i = 0; i < 6; i++)
		{
			if (m_Virtual_mapPieces[i].At(move.index))
				type = (Piece)i;
			if (m_Virtual_mapPieces[i].At(direction))
				enemyType = (Piece)i;
		}

		if (type == NONE)
			return MOVEERROR;

		m_Virtual_Pieces.Set(move.index, false);
		m_Virtual_mapPieces[type].Set(move.index, false);

		if (m_PlayerToPlay == WHITE)
		{
			m_Virtual_WhitePieces.Set(move.index, false);
			m_Virtual_mapWhitePieces[type].Set(move.index, false);

			m_Virtual_WhitePieces.Set(direction, true);
			m_Virtual_mapWhitePieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_Virtual_mapPieces[type].Set(direction, true);
					m_Virtual_mapPieces[enemyType].Set(direction, false);
				}

				m_Virtual_BlackPieces.Set(direction, false);
				m_Virtual_mapBlackPieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Virtual_Pieces.Set(direction, true);
				m_Virtual_mapPieces[type].Set(direction, true);
			}

			//White AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				m_Virtual_Pieces.Set(direction - 8, false);
				m_Virtual_mapPieces[PAWN].Set(direction - 8, false);
				m_Virtual_BlackPieces.Set(direction - 8, false);
				m_Virtual_mapBlackPieces[PAWN].Set(direction - 8, false);
			}
		}
		else
		{
			m_Virtual_BlackPieces.Set(move.index, false);
			m_Virtual_mapBlackPieces[type].Set(move.index, false);

			m_Virtual_BlackPieces.Set(direction, true);
			m_Virtual_mapBlackPieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_Virtual_mapPieces[type].Set(direction, true);
					m_Virtual_mapPieces[enemyType].Set(direction, false);
				}

				m_Virtual_WhitePieces.Set(direction, false);
				m_Virtual_mapWhitePieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Virtual_Pieces.Set(direction, true);
				m_Virtual_mapPieces[type].Set(direction, true);
			}

			//Black AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				m_Virtual_Pieces.Set(direction + 8, false);
				m_Virtual_mapPieces[PAWN].Set(direction + 8, false);
				m_Virtual_WhitePieces.Set(direction + 8, false);
				m_Virtual_mapWhitePieces[PAWN].Set(direction + 8, false);
			}
		}

		return SUCCESS;
	}

	Board::KingStatus Board::GetVirtualKingStatus(Color playerColor) const
	{
		if (playerColor == WHITE)
		{
			int kingIndex = std::countr_zero(m_Virtual_mapWhitePieces[KING].Data());
			
			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			BitBoard enemyQueenBishop = m_Virtual_mapBlackPieces[QUEEN] + m_Virtual_mapBlackPieces[BISHOP];
			BitBoard enemyQueenRook = m_Virtual_mapBlackPieces[QUEEN] + m_Virtual_mapBlackPieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y))
						break;

				if (enemyQueenRook.At(kingIndex + y))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y))
					break;
			}

			if ((m_Virtual_mapBlackPieces[PAWN].At(kingIndex + 7) && left) || (m_Virtual_mapBlackPieces[PAWN].At(kingIndex + 9) && right))
				return CHECKED;

			if (up >= 2)
			{
				if (left && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					return CHECKED;
			}
			if (down >= 2)
			{
				if (left && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					return CHECKED;
			}
			if (left >= 2)
			{
				if (down && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					return CHECKED;
				if (up && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					return CHECKED;
			}
			if (right >= 2)
			{
				if (down && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					return CHECKED;
				if (up && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					return CHECKED;
			}

			if (up)
			{
				if (m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[2]))
					return CHECKED;
				if (left && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[1]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[3]))
					return CHECKED;
			}
			if (down)
			{
				if (m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[2]))
					return CHECKED;
				if (left && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[1]))
					return CHECKED;
			}
			if (left && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[0]))
				return CHECKED;
			if (right && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[0]))
				return CHECKED;
		}
		else
		{
			int kingIndex = std::countr_zero(m_Virtual_mapBlackPieces[KING].Data());

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			BitBoard enemyQueenBishop = m_Virtual_mapWhitePieces[QUEEN] + m_Virtual_mapWhitePieces[BISHOP];
			BitBoard enemyQueenRook = m_Virtual_mapWhitePieces[QUEEN] + m_Virtual_mapWhitePieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y))
					break;
			}

			if ((m_Virtual_mapWhitePieces[PAWN].At(kingIndex - 9) && left) || (m_Virtual_mapWhitePieces[PAWN].At(kingIndex - 7) && right))
				return CHECKED;

			if (up >= 2)
			{
				if (left && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					return CHECKED;
			}
			if (down >= 2)
			{
				if (left && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					return CHECKED;
			}
			if (left >= 2)
			{
				if (down && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					return CHECKED;
				if (up && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					return CHECKED;
			}
			if (right >= 2)
			{
				if (down && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					return CHECKED;
				if (up && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					return CHECKED;
			}

			if (up)
			{
				if (m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[2]))
					return CHECKED;
				if (left && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[1]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[3]))
					return CHECKED;
			}
			if (down)
			{
				if (m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[2]))
					return CHECKED;
				if (left && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[1]))
					return CHECKED;
			}
			if (left && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[0]))
				return CHECKED;
			if (right && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[0]))
				return CHECKED;

		}
		
		return SECURE;
	}

	bool Board::IsMoveKingSecured(Move move) const
	{
		UpdateVitualValues();
		VirtualMakeMove(move);

		return GetVirtualKingStatus(m_PlayerToPlay) == SECURE;
	}

	void Board::FindPawnMoves(std::vector<Move>& moves) const
	{
		auto& whitePawns = m_mapWhitePieces[PAWN];
		auto& blackPawns = m_mapBlackPieces[PAWN];

		auto allPieces = m_WhitePieces + m_BlackPieces;

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 8; i < 56; i++)
			{
				if (whitePawns.At(i))
				{
					if (!allPieces.At(i + PawnMoves[0]))
					{
						moves.emplace_back(i, PawnMoves[0]);

						if (i < 16 && !allPieces.At(i + PawnMoves[1]))
							moves.emplace_back(i, PawnMoves[1]);
					}

					if (i % 8 < 7 && (m_BlackPieces.At(i + PawnMoves[2]) || (i + PawnMoves[2]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, PawnMoves[2]);

					if (i % 8 > 0 && (m_BlackPieces.At(i + PawnMoves[3]) || (i + PawnMoves[3]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, PawnMoves[3]);
				}
			}
		}
		else
		{
			for (int i = 8; i < 56; i++)
			{
				if (blackPawns.At(i))
				{
					if (!allPieces.At(i - PawnMoves[0]))
					{
						moves.emplace_back(i, -PawnMoves[0]);

						if (i > 47 && !allPieces.At(i - PawnMoves[1]))
							moves.emplace_back(i, -PawnMoves[1]);
					}

					if (i % 8 > 0 && (m_WhitePieces.At(i - PawnMoves[2]) || (i - PawnMoves[2]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, -PawnMoves[2]);

					if (i % 8 < 7 && (m_WhitePieces.At(i - PawnMoves[3]) || (i - PawnMoves[3]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, -PawnMoves[3]);
				}
			}
		}
	}

	void Board::FindKngihtMoves(std::vector<Move>& moves) const
	{
		auto& whiteKngihts = m_mapWhitePieces[KNIGHT];
		auto& blackKngihts = m_mapBlackPieces[KNIGHT];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				bool ok_right = right >= 2;
				bool ok_left = left >= 2;
				bool ok_up = up >= 2;
				bool ok_down = down >= 2;

				if (whiteKngihts.At(i))
				{
					if (ok_up)
					{
						if (left && !m_WhitePieces.At(i + KnightMoves[3]))
							moves.emplace_back(i, KnightMoves[3]);
						if (right && !m_WhitePieces.At(i + KnightMoves[2]))
							moves.emplace_back(i, KnightMoves[2]);
					}
					if (ok_down)
					{
						if (left && !m_WhitePieces.At(i - KnightMoves[2]))
							moves.emplace_back(i, -KnightMoves[2]);
						if (right && !m_WhitePieces.At(i - KnightMoves[3]))
							moves.emplace_back(i, -KnightMoves[3]);
					}
					if (ok_left)
					{
						if (down && !m_WhitePieces.At(i - KnightMoves[1]))
							moves.emplace_back(i, -KnightMoves[1]);
						if (up && !m_WhitePieces.At(i + KnightMoves[0]))
							moves.emplace_back(i, KnightMoves[0]);
					}
					if (ok_right)
					{
						if (down && !m_WhitePieces.At(i - KnightMoves[0]))
							moves.emplace_back(i, -KnightMoves[0]);
						if (up && !m_WhitePieces.At(i + KnightMoves[1]))
							moves.emplace_back(i, KnightMoves[1]);
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				bool ok_right = right >= 2;
				bool ok_left = left >= 2;
				bool ok_up = up >= 2;
				bool ok_down = down >= 2;

				if (blackKngihts.At(i))
				{
					if (ok_up)
					{
						if (left && !m_BlackPieces.At(i + KnightMoves[3]))
							moves.emplace_back(i, KnightMoves[3]);
						if (right && !m_BlackPieces.At(i + KnightMoves[2]))
							moves.emplace_back(i, KnightMoves[2]);
					}
					if (ok_down)
					{
						if (left && !m_BlackPieces.At(i - KnightMoves[2]))
							moves.emplace_back(i, -KnightMoves[2]);
						if (right && !m_BlackPieces.At(i - KnightMoves[3]))
							moves.emplace_back(i, -KnightMoves[3]);
					}
					if (ok_left)
					{
						if (down && !m_BlackPieces.At(i - KnightMoves[1]))
							moves.emplace_back(i, -KnightMoves[1]);
						if (up && !m_BlackPieces.At(i + KnightMoves[0]))
							moves.emplace_back(i, KnightMoves[0]);
					}
					if (ok_right)
					{
						if (down && !m_BlackPieces.At(i - KnightMoves[0]))
							moves.emplace_back(i, -KnightMoves[0]);
						if (up && !m_BlackPieces.At(i + KnightMoves[1]))
							moves.emplace_back(i, KnightMoves[1]);
					}
				}
			}
		}

		
	}

	void Board::FindBishopMoves(std::vector<Move>& moves) const
	{
		auto& whiteBishops = m_mapWhitePieces[BISHOP];
		auto& blackBishops = m_mapBlackPieces[BISHOP];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteBishops.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_BlackPieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_BlackPieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_BlackPieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_BlackPieces.At(i - y * 7))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackBishops.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_WhitePieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_WhitePieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_WhitePieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_WhitePieces.At(i - y * 7))
							break;
					}
				}
			}
		}
	}

	void Board::FindRookMoves(std::vector<Move>& moves) const
	{
		auto& whiteRooks = m_mapWhitePieces[ROOK];
		auto& blackRooks = m_mapBlackPieces[ROOK];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteRooks.At(i))
				{
					for (int y = 1; y < up + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_BlackPieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_WhitePieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_BlackPieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_BlackPieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_WhitePieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_BlackPieces.At(i + y))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackRooks.At(i))
				{
					for (int y = 1; y < up + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_WhitePieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_BlackPieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_WhitePieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_WhitePieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_BlackPieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_WhitePieces.At(i + y))
							break;
					}
				}
			}
		}

		
	}

	void Board::FindQueenMoves(std::vector<Move>& moves) const
	{
		auto& whiteQueens = m_mapWhitePieces[QUEEN];
		auto& blackQueens = m_mapBlackPieces[QUEEN];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteQueens.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_BlackPieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_BlackPieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_BlackPieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_BlackPieces.At(i - y * 7))
							break;
					}

					for (int y = 1; y < up + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_BlackPieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_WhitePieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_BlackPieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_BlackPieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_WhitePieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_BlackPieces.At(i + y))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackQueens.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_WhitePieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_WhitePieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_WhitePieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_WhitePieces.At(i - y * 7))
							break;
					}

					for (int y = 1; y < up + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_WhitePieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_BlackPieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_WhitePieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_WhitePieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_BlackPieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_WhitePieces.At(i + y))
							break;
					}
				}
			}
		}		
	}

	void Board::FindKingMoves(std::vector<Move>& moves) const
	{
		auto allPieces = m_WhitePieces + m_BlackPieces;

		if (m_PlayerToPlay == WHITE)
		{
			int i = (int)std::log2(m_mapWhitePieces[KING].Data());

			int yLevel = i / 8;
			int xLevel = i % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 1;
			bool ok_left = left >= 1;
			bool ok_up = up >= 1;
			bool ok_down = down >= 1;

			
			if (ok_up)
			{
				if (!m_WhitePieces.At(i + KingMoves[2]))
					moves.emplace_back(i, KingMoves[2]);
				if (ok_left && !m_WhitePieces.At(i + KingMoves[1]))
					moves.emplace_back(i, KingMoves[1]);
				if (ok_right && !m_WhitePieces.At(i + KingMoves[3]))
					moves.emplace_back(i, KingMoves[3]);
			}
			if (ok_down)
			{
				if (!m_WhitePieces.At(i - KingMoves[2]))
					moves.emplace_back(i, -KingMoves[2]);
				if (ok_left && !m_WhitePieces.At(i - KingMoves[3]))
					moves.emplace_back(i, -KingMoves[3]);
				if (ok_right && !m_WhitePieces.At(i - KingMoves[1]))
					moves.emplace_back(i, -KingMoves[1]);
			}
			if (ok_left && !m_WhitePieces.At(i - KingMoves[0]))
				moves.emplace_back(i, -KingMoves[0]);
			if (ok_right && !m_WhitePieces.At(i + KingMoves[0]))
				moves.emplace_back(i, KingMoves[0]);

			if (m_K)
			{
				if (!allPieces.At(4 + 1) && !allPieces.At(4 + 2) && m_mapWhitePieces[ROOK].At(4 + 3) && IsMoveKingSecured({4, 0}) && IsMoveKingSecured({4, 1}))
					moves.emplace_back(4, 2);
			}
			if (m_Q)
			{
				if (!allPieces.At(4 - 1) && !allPieces.At(4 - 2) && !allPieces.At(4 - 3) && m_mapWhitePieces[ROOK].At(4 - 4) && IsMoveKingSecured({ 4, 0 }) && IsMoveKingSecured({ 4, -1 }))
					moves.emplace_back(4, -2);
			}
		}
		else
		{
			int i = (int)std::log2(m_mapBlackPieces[KING].Data());

			int yLevel = i / 8;
			int xLevel = i % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 1;
			bool ok_left = left >= 1;
			bool ok_up = up >= 1;
			bool ok_down = down >= 1;

			if (ok_up)
			{
				if (!m_BlackPieces.At(i + KingMoves[2]))
					moves.emplace_back(i, KingMoves[2]);
				if (ok_left && !m_BlackPieces.At(i + KingMoves[1]))
					moves.emplace_back(i, KingMoves[1]);
				if (ok_right && !m_BlackPieces.At(i + KingMoves[3]))
					moves.emplace_back(i, KingMoves[3]);
			}
			if (ok_down)
			{
				if (!m_BlackPieces.At(i - KingMoves[2]))
					moves.emplace_back(i, -KingMoves[2]);
				if (ok_left && !m_BlackPieces.At(i - KingMoves[3]))
					moves.emplace_back(i, -KingMoves[3]);
				if (ok_right && !m_BlackPieces.At(i - KingMoves[1]))
					moves.emplace_back(i, -KingMoves[1]);
			}
			if (ok_left && !m_BlackPieces.At(i - KingMoves[0]))
				moves.emplace_back(i, -KingMoves[0]);
			if (ok_right && !m_BlackPieces.At(i + KingMoves[0]))
					moves.emplace_back(i, KingMoves[0]);

			if (m_k)
			{
				if (!allPieces.At(60 + 1) && !allPieces.At(60 + 2) && m_mapBlackPieces[ROOK].At(60 + 3) && IsMoveKingSecured({ 60, 0 }) && IsMoveKingSecured({ 60, 1 }))
					moves.emplace_back(60, 2);
			}
			if (m_q)
			{
				if (!allPieces.At(60 - 1) && !allPieces.At(60 - 2) && !allPieces.At(60 - 3) && m_mapBlackPieces[ROOK].At(60 - 4) && IsMoveKingSecured({ 60, 0 }) && IsMoveKingSecured({ 60, -1 }))
					moves.emplace_back(60, -2);
			}
		}
	}

	bool Board::IsBoardValid() const
	{
		int allWhitepecies = 0;
		int allBlackpecies = 0;
		
		for (int i = 0; i < 64; ++i)
		{
			if (GetPieceType(i) != NONE)
			{
				if (GetPieceColor(i) == WHITE)
					allWhitepecies += 1;
				else
					allBlackpecies += 1;
			}
		}

		if (allWhitepecies > 16 || allBlackpecies > 16)
			return false;

		int WkingAmount = 0, BkingAmount = 0;

		for (int i = 0; i < 64; ++i)
		{
			if (GetPieceType(i) == KING)
			{
				if (GetPieceColor(i) == WHITE)
					WkingAmount += 1;
				else
					BkingAmount += 1;
			}
		}

		if (WkingAmount != 1 || BkingAmount != 1)
			return false;

		int kingsDistance = std::abs((int)std::log2(m_mapWhitePieces[KING].Data()) - (int)std::log2(m_mapBlackPieces[KING].Data()));

		if (kingsDistance == 1 || kingsDistance == 7 || kingsDistance == 8 || kingsDistance == 9)
			return false;

		UpdateVitualValues();

		if (GetVirtualKingStatus((m_PlayerToPlay == WHITE ? BLACK : WHITE)) != SECURE)
			return false;
		
		return true;
	}

	void Board::SetKRoke(bool K)
	{
		if (m_K != K)
			m_Hash ^= m_Hash ^= s_rhn_64[s_stages[13]];

		m_K = K;
	}

	void Board::SetQRoke(bool Q)
	{
		if (m_Q != Q)
			m_Hash ^= m_Hash ^= s_rhn_64[s_stages[14]];

		m_Q = Q;
	}

	void Board::SetkRoke(bool k)
	{
		if (m_k != k)
			m_Hash ^= m_Hash ^= s_rhn_64[s_stages[15]];

		m_k = k;
	}

	void Board::SetqRoke(bool q)
	{
		if (m_q != q)
			m_Hash ^= m_Hash ^= s_rhn_64[s_stages[16]];

		m_q = q;
	}

	void Board::SwapPlayerToPlay() 
	{
		m_Hash ^= s_rhn_64[s_stages[12]];

		m_PlayerToPlay = m_PlayerToPlay == WHITE ? BLACK : WHITE; 
	}

	void Board::SetLastMoveIndex(int LastMoveIndex) 
	{
		if (m_LastMovedPieceIndex != -1)
			m_Hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];

		m_LastMovedPieceIndex = LastMoveIndex; 
		m_Hash ^= s_rhn_64[s_stages[17] + m_LastMovedPieceIndex % 8];
	}

	std::string Board::ConvertUCIMoveToPGNMove(const std::string& uciMove) const
	{
		Move moveCore;
		Piece promotedType;
		ConvertUCIMoveToCoreMove(moveCore, promotedType, uciMove);
		return ConvertCoreMoveToPGNMove(moveCore, promotedType);
	}

	std::string Board::ConvertPGNMoveToUCIMove(const std::string& pgnMove) const
	{
		Move moveCore;
		Piece promotedType;
		ConvertPGNMoveToCoreMove(moveCore, promotedType, pgnMove);
		return ConvertCoreMoveToUCIMove(moveCore, promotedType);
	}

	std::string Board::ConvertCLDMoveToUCIMove(std::pair<uint8_t, uint8_t> cldMove) const
	{
		Move moveCore;
		Piece promotedType;
		ConvertCLDMoveToCoreMove(moveCore, promotedType, cldMove);
		return ConvertCoreMoveToUCIMove(moveCore, promotedType);
	}

	std::string Board::ConvertCLDMoveToPGNMove(std::pair<uint8_t, uint8_t> cldMove)
	{
		static const char* pieceNames = "NBRQPK";

		std::string moveToReturn = "";

		uint8_t moveDir = cldMove.first & 0b00111111;
		uint8_t movePartOne = (cldMove.second & 0b11100000) >> 5;
		uint8_t movePartSecond = (cldMove.second & 0b00011100) >> 2;
		uint8_t movePartThird = cldMove.second & 0b00000011;

		char DirX = 'a' + (moveDir >> 3);
		char DirY = '1' + (moveDir & 0b00000111);

		if (movePartOne == 0)
		{
			if ((moveDir >> 3) != movePartSecond)
			{
				moveToReturn += ('a' + movePartSecond);
				moveToReturn += 'x';
			}

			moveToReturn += DirX;
			moveToReturn += DirY;

			if (DirY == '1' || DirY == '8')
			{
				moveToReturn += '=';
				moveToReturn += pieceNames[movePartThird];
			}
		}
		else if (movePartOne == 1)
		{
			if (movePartSecond == 0)
			{
				moveToReturn += 'K';
				moveToReturn += DirX;
				moveToReturn += DirY;
			}
			else if (movePartSecond == 1)
			{
				moveToReturn += 'K';
				moveToReturn += 'x';
				moveToReturn += DirX;
				moveToReturn += DirY;
			}
			else if (movePartSecond == 2)
			{
				moveToReturn += pieceNames[movePartThird];
				moveToReturn += DirX;
				moveToReturn += DirY;
			}
			else if (movePartSecond == 3)
			{
				moveToReturn += pieceNames[movePartThird];
				moveToReturn += 'x';
				moveToReturn += DirX;
				moveToReturn += DirY;
			}
			else if (movePartSecond == 4)
			{
				moveToReturn += "O-O";
			}
			else if (movePartSecond == 5)
			{
				moveToReturn += "O-O-O";
			}
		}
		else if (movePartOne == 2)
		{
			moveToReturn += pieceNames[movePartThird];
			moveToReturn += ('a' + movePartSecond);
			moveToReturn += DirX;
			moveToReturn += DirY;
		}
		else if (movePartOne == 3)
		{
			moveToReturn += pieceNames[movePartThird];
			moveToReturn += ('a' + movePartSecond);
			moveToReturn += 'x';
			moveToReturn += DirX;
			moveToReturn += DirY;
		}
		else if (movePartOne == 4)
		{
			moveToReturn += pieceNames[movePartThird];
			moveToReturn += ('1' + movePartSecond);
			moveToReturn += DirX;
			moveToReturn += DirY;
		}
		else if (movePartOne == 5)
		{
			moveToReturn += pieceNames[movePartThird];
			moveToReturn += ('1' + movePartSecond);
			moveToReturn += 'x';
			moveToReturn += DirX;
			moveToReturn += DirY;
		}
		else if (movePartOne == 6)
		{
			static const int converter[] = { -2, -1, 1, 2 };
			char pN;
			char posX;
			char posY;

			if (movePartThird == 0)
			{
				int converted = converter[movePartSecond % 4];
				posX = DirX + converted;

				converted = (std::abs(converted) == 2 ? 1 : 2);

				if (movePartSecond / 4 == 0)
					posY = DirY - converted;
				else
					posY = DirY + converted;

				pN = pieceNames[movePartThird];
			}
			else if (movePartThird == 1 || movePartThird == 2)
			{
				int converted = converter[movePartSecond % 4];
				posX = DirX + converted;

				if (movePartSecond / 4 == 0)
					posY = DirY - converted;
				else
					posY = DirY + converted;

				pN = pieceNames[movePartThird];
			}
			else
			{
				int pos3 = movePartSecond % 4;

				if (pos3 == 0)
				{
					posX = DirX - 3;
					posY = DirY - 3;
				}
				else if (pos3 == 1)
				{
					posX = DirX + 3;
					posY = DirY - 3;
				}
				else if (pos3 == 2)
				{
					posX = DirX - 3;
					posY = DirY + 3;
				}
				else
				{
					posX = DirX + 3;
					posY = DirY + 3;
				}

				if (movePartSecond / 4 == 0)
					pN = 'B';
				else
					pN = 'Q';
			}

			moveToReturn += pN;
			moveToReturn += posX;
			moveToReturn += posY;
			moveToReturn += DirX;
			moveToReturn += DirY;
		}
		else if (movePartOne == 7)
		{
			static const int converter[] = { -2, -1, 1, 2 };
			char pN;
			char posX;
			char posY;

			if (movePartThird == 0)
			{
				int converted = converter[movePartSecond % 4];
				posX = DirX + converted;

				converted = (std::abs(converted) == 2 ? 1 : 2);

				if (movePartSecond / 4 == 0)
					posY = DirY - converted;
				else
					posY = DirY + converted;

				pN = pieceNames[movePartThird];
			}
			else if (movePartThird == 1 || movePartThird == 2)
			{
				int converted = converter[movePartSecond % 4];
				posX = DirX + converted;

				if (movePartSecond / 4 == 0)
					posY = DirY - converted;
				else
					posY = DirY + converted;

				pN = pieceNames[movePartThird];
			}
			else
			{
				int pos3 = movePartSecond % 4;

				if (pos3 == 0)
				{
					posX = DirX - 3;
					posY = DirY - 3;
				}
				else if (pos3 == 1)
				{
					posX = DirX + 3;
					posY = DirY - 3;
				}
				else if (pos3 == 2)
				{
					posX = DirX - 3;
					posY = DirY + 3;
				}
				else
				{
					posX = DirX + 3;
					posY = DirY + 3;
				}

				if (movePartSecond / 4 == 0)
					pN = 'B';
				else
					pN = 'Q';
			}

			moveToReturn += pN;
			moveToReturn += posX;
			moveToReturn += posY;
			moveToReturn += 'x';
			moveToReturn += DirX;
			moveToReturn += DirY;
		}

		return moveToReturn;
	}

	std::pair<uint8_t, uint8_t> Board::ConvertUCIMoveToCLDMove(const std::string& uciMove) const
	{
		Move moveCore;
		Piece promotedType;
		ConvertUCIMoveToCoreMove(moveCore, promotedType, uciMove);
		return ConvertCoreMoveToCLDMove(moveCore, promotedType);
	}

	std::pair<uint8_t, uint8_t> Board::ConvertPGNMoveToCLDMove(const std::string& pgnMove)
	{
		const uint8_t converter[] = { 0, 1, 0, 2, 3 };

		uint8_t firstPart, secondPart;

		size_t moveStart = -1, moveDirStart = -1, yPosIndex = -1, xPosIndex = -1;
		bool xPos = false, yPos = false, taking = false, Prom = false;

		bool bigRoke = pgnMove.find("O-O-O") != std::string::npos || pgnMove.find("0-0-0") != std::string::npos;
		bool smallRoke = pgnMove.find("O-O") != std::string::npos || pgnMove.find("0-0") != std::string::npos;

		if (bigRoke)
		{
			firstPart = 0;
			secondPart = 0b00110100;
		}
		else if (smallRoke)
		{
			firstPart = 0;
			secondPart = 0b00110000;
		}
		else
		{
			moveStart = pgnMove.find(' ');

			if (moveStart == std::string::npos)
				moveStart = 0;
			else
				moveStart += 1;

			moveDirStart = moveStart;

			for (int k = pgnMove.size() - 1; k > moveStart; k--)
			{
				if (pgnMove[k] >= '1' && pgnMove[k] <= '8')
				{
					moveDirStart = k - 1;
					break;
				}
			}

			Prom = pgnMove.find('=') != std::string::npos;
			taking = pgnMove.find('x') != std::string::npos;
			uint8_t dirX = pgnMove[moveDirStart] - 'a';
			uint8_t dirY = pgnMove[moveDirStart + 1] - '1';

			firstPart = (dirX << 3) | dirY;

			if (pgnMove[moveStart] > 'A' && pgnMove[moveStart] < 'Z')
			{
				int checkIfPos = moveDirStart - moveStart;

				if (taking)
					checkIfPos -= 1;

				for (int k = 1; k < checkIfPos; k++)
				{
					char c = pgnMove[k + moveStart];
					if (c >= 'a' && c <= 'h')
					{
						xPosIndex = c - 'a';
						xPos = true;
					}
					else if (c >= '1' && c <= '8')
					{
						yPosIndex = c - '1';
						yPos = true;
					}
				}

				if (!xPos && !yPos)
				{
					if (pgnMove[moveStart] == 'K')
					{
						if (taking)
							secondPart = 0b00100100;
						else
							secondPart = 0b00100000;
					}
					else if (pgnMove[moveStart] == 'N')
					{
						if (taking)
							secondPart = 0b00101100;
						else
							secondPart = 0b00101000;
					}
					else if (pgnMove[moveStart] == 'B')
					{
						if (taking)
							secondPart = 0b00101101;
						else
							secondPart = 0b00101001;
					}
					else if (pgnMove[moveStart] == 'R')
					{
						if (taking)
							secondPart = 0b00101110;
						else
							secondPart = 0b00101010;
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						if (taking)
							secondPart = 0b00101111;
						else
							secondPart = 0b00101011;
					}
				}
				else if (xPos && !yPos && !taking)
				{
					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b01000000 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						secondPart = 0b01000001 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'R')
					{
						secondPart = 0b01000010 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						secondPart = 0b01000011 | (xPosIndex << 2);
					}
				}
				else if (xPos && !yPos && taking)
				{
					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b01100000 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						secondPart = 0b01100001 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'R')
					{
						secondPart = 0b01100010 | (xPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						secondPart = 0b01100011 | (xPosIndex << 2);
					}
				}
				else if (!xPos && yPos && !taking)
				{
					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b10000000 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						secondPart = 0b10000001 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'R')
					{
						secondPart = 0b10000010 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						secondPart = 0b10000011 | (yPosIndex << 2);
					}
				}
				else if (!xPos && yPos && taking)
				{
					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b10100000 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						secondPart = 0b01010001 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'R')
					{
						secondPart = 0b10100010 | (yPosIndex << 2);
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						secondPart = 0b10100011 | (yPosIndex << 2);
					}
				}
				else if (xPos && yPos && !taking)
				{
					int xDiff = dirX - xPosIndex;
					int yDiff = dirY - yPosIndex;

					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b11000000 | ((converter[xDiff + 2] + (0 ? yDiff > 0 : 4)) << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11000010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11000110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11001010;
							}
							else
							{
								secondPart = 0b11001110;
							}
						}
						else
						{
							secondPart = 0b11000001 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
						}
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11010010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11010110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11011010;
							}
							else
							{
								secondPart = 0b11011110;
							}
						}
						else
						{
							secondPart = 0b11000011 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
						}
					}
				}
				else if (xPos && yPos && taking)
				{
					int xDiff = dirX - xPosIndex;
					int yDiff = dirY - yPosIndex;

					if (pgnMove[moveStart] == 'N')
					{
						secondPart = 0b11100000 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					}
					else if (pgnMove[moveStart] == 'B')
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11100010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11100110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11101010;
							}
							else
							{
								secondPart = 0b11101110;
							}
						}
						else
						{
							secondPart = 0b11100001 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
						}
					}
					else if (pgnMove[moveStart] == 'Q')
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11110010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11110110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11111010;
							}
							else
							{
								secondPart = 0b11111110;
							}
						}
						else
						{
							secondPart = 0b11100011 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
						}
					}
				}
			}
			else
			{
				if (taking)
				{
					secondPart = 0b00000000 | ((pgnMove[moveStart] - 'a') << 2);
				}
				else
				{
					secondPart = 0b00000000 | (dirX << 2);
				}

				if (Prom)
				{
					char type = pgnMove[moveDirStart + 3];

					if (type == 'B')
						secondPart |= 1;
					else if (type == 'R')
						secondPart |= 2;
					else if (type == 'Q')
						secondPart |= 3;
				}
			}
		}

		return { firstPart, secondPart };
	}

	std::string Board::ConvertCoreMoveToUCIMove(const Board::Move& move, Piece promotedType) const
	{
		std::string uciMove;

		uciMove += s_strMoveIndexX[move.index % 8];
		uciMove += s_strMoveIndexY[move.index / 8];
		uciMove += s_strMoveIndexX[(move.index + move.move) % 8];
		uciMove += s_strMoveIndexY[(move.index + move.move) / 8];

		if (promotedType != NONE)
			uciMove += s_strMoveTypesSmall[(int)promotedType];

		return uciMove;
	}

	std::string Board::ConvertCoreMoveToPGNMove(const Board::Move& move, Piece promotedType) const
	{
		std::string strmove;

		Piece pieceToMove = GetPieceType(move.index);
		Piece pieceOnDirection = GetPieceType(move.index + move.move);

		if (pieceToMove == NONE)
		{
			strmove = "";
			return strmove;
		}

		if (m_PlayerToPlay== WHITE)
			strmove += (std::to_string(m_BlackMovesCounter) + ". ");
		if (pieceToMove == KING && std::abs(move.move) == 2.0f)
		{
			//roke
			if (move.move > 0)
				strmove += "O-O";
			else
				strmove += "O-O-O";
		}
		else
		{
			if (pieceToMove != PAWN)
				strmove += s_strMoveTypes[(int)pieceToMove];
			if (pieceToMove == PAWN && pieceOnDirection != NONE)
				strmove += s_strMoveIndexX[move.index % 8];

			std::vector<int> possibleIndex;
			std::vector<Board::Move> possibleMoves;
			GetAvailableMoves(possibleMoves, pieceToMove);

			for (auto& m : possibleMoves)
			{
				if ((m.index + m.move) == (move.index + move.move) && m.index != move.index)
					possibleIndex.emplace_back(m.index);
			}

			bool showIndexX = false, showIndexY = false;

			if (possibleIndex.size() > 0 && pieceToMove != PAWN)
			{
				for (auto& pi : possibleIndex)
				{
					if (pi % 8 == move.index % 8)
						showIndexY = true;
					if (pi / 8 == move.index / 8)
						showIndexX = true;
				}

				if (!showIndexX && !showIndexY)
					showIndexX = true;
			}

			if (showIndexX)
				strmove += s_strMoveIndexX[move.index % 8];
			if (showIndexY)
				strmove += s_strMoveIndexY[move.index / 8];

			if (pieceOnDirection != NONE)
				strmove += 'x';

			strmove += s_strMoveIndexX[(move.index + move.move) % 8];
			strmove += s_strMoveIndexY[(move.index + move.move) / 8];
		}

		if (promotedType != NONE && pieceToMove == PAWN && ((move.index + move.move) / 8 == 0 || (move.index + move.move) / 8 == 7))
		{
			strmove += '=';
			strmove += s_strMoveTypes[(int)promotedType];
		}

		Board boardCopy = *this;
		boardCopy.MakeMove(move, promotedType);
		auto kingSecurity = boardCopy.GetKingStatus();

		if (kingSecurity == Board::CHECKED)
			strmove += '+';
		else if (kingSecurity == Board::MATED)
			strmove += '#';

		return strmove;
	}

	std::pair<uint8_t, uint8_t> Board::ConvertCoreMoveToCLDMove(const Board::Move& move, Piece promotedType) const
	{
		const uint8_t converter[] = { 0, 1, 0, 2, 3 };

		uint8_t firstPart = 0, secondPart = 0;

		bool xPos = false, yPos = false, taking = false;
		Piece pieceToMove = GetPieceType(move.index);
		Piece pieceOnDirection = GetPieceType(move.index + move.move);

		bool bigRoke = pieceToMove == KING && move.move == -2.0f;
		bool smallRoke = pieceToMove == KING && move.move == 2.0f;

		if (bigRoke)
		{
			firstPart = 0;
			secondPart = 0b00110100;
		}
		else if (smallRoke)
		{
			firstPart = 0;
			secondPart = 0b00110000;
		}
		else
		{
			taking = pieceOnDirection != NONE;

			uint8_t dirX = (move.index + move.move) % 8;
			uint8_t dirY = (move.index + move.move) / 8;

			firstPart = (dirX << 3) | dirY;

			if (pieceToMove != PAWN)
			{
				std::vector<int> possibleIndex;
				std::vector<Board::Move> possibleMoves;
				GetAvailableMoves(possibleMoves, pieceToMove);

				for (auto& m : possibleMoves)
				{
					if ((m.index + m.move) == (move.index + move.move) && m.index != move.index)
						possibleIndex.emplace_back(m.index);
				}

				if (!possibleIndex.empty())
				{
					for (auto& pi : possibleIndex)
					{
						if (pi % 8 == move.index % 8)
							yPos = true;
						if (pi / 8 == move.index / 8)
							xPos = true;
					}

					if (!xPos && !yPos)
						xPos = true;
				}

				if (!xPos && !yPos)
				{
					if (pieceToMove == KING)
					{
						if (taking)
							secondPart = 0b00100100;
						else
							secondPart = 0b00100000;
					}
					else if (pieceToMove == KNIGHT)
					{
						if (taking)
							secondPart = 0b00101100;
						else
							secondPart = 0b00101000;
					}
					else if (pieceToMove == BISHOP)
					{
						if (taking)
							secondPart = 0b00101101;
						else
							secondPart = 0b00101001;
					}
					else if (pieceToMove == ROOK)
					{
						if (taking)
							secondPart = 0b00101110;
						else
							secondPart = 0b00101010;
					}
					else if (pieceToMove == QUEEN)
					{
						if (taking)
							secondPart = 0b00101111;
						else
							secondPart = 0b00101011;
					}
				}
				else if (xPos && !yPos && !taking)
				{
					secondPart = 0b01000000 | (pieceToMove - 1) | ((move.index % 8) << 2);
				}
				else if (xPos && !yPos && taking)
				{
					secondPart = 0b01100000 | (pieceToMove - 1) | ((move.index % 8) << 2);
				}
				else if (!xPos && yPos && !taking)
				{
					secondPart = 0b10000000 | (pieceToMove - 1) | ((move.index / 8) << 2);
				}
				else if (!xPos && yPos && taking)
				{
					secondPart = 0b10100000 | (pieceToMove - 1) | ((move.index / 8) << 2);
				}
				else if (xPos && yPos && !taking)
				{
					int xDiff = move.index % 8;
					int yDiff = move.index / 8;

					if (pieceToMove == KNIGHT)
						secondPart = 0b11000000 | ((converter[xDiff + 2] + (0 ? yDiff > 0 : 4)) << 2);
					else if (pieceToMove == BISHOP)
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11000010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11000110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11001010;
							}
							else
							{
								secondPart = 0b11001110;
							}
						}
						else
							secondPart = 0b11000001 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					}
					else if (pieceToMove == QUEEN)
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11010010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11010110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11011010;
							}
							else
							{
								secondPart = 0b11011110;
							}
						}
						else
							secondPart = 0b11000011 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					}
				}
				else if (xPos && yPos && taking)
				{
					int xDiff = move.index % 8;
					int yDiff = move.index / 8;

					if (pieceToMove == KNIGHT)
						secondPart = 0b11100000 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					else if (pieceToMove == BISHOP)
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11100010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11100110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11101010;
							}
							else
							{
								secondPart = 0b11101110;
							}
						}
						else
							secondPart = 0b11100001 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					}
					else if (pieceToMove == QUEEN)
					{
						if (std::abs(xDiff) > 2)
						{
							if (xDiff < 0 && yDiff < 0)
							{
								secondPart = 0b11110010;
							}
							else if (xDiff > 0 && yDiff < 0)
							{
								secondPart = 0b11110110;
							}
							else if (xDiff < 0 && yDiff > 0)
							{
								secondPart = 0b11111010;
							}
							else
							{
								secondPart = 0b11111110;
							}
						}
						else
							secondPart = 0b11100011 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
					}
				}
			}
			else
			{
				if (move.index % 8 != (move.index + move.move) % 8)
					taking = true;

				if (taking)
					secondPart = 0b00000000 | ((move.index % 8) << 2);
				else
					secondPart = 0b00000000 | (dirX << 2);

				if (((move.index + move.move) / 8 == 0 || (move.index + move.move) / 8 == 7))
				{
					if (promotedType == BISHOP)
						secondPart |= 1;
					else if (promotedType == ROOK)
						secondPart |= 2;
					else if (promotedType == QUEEN)
						secondPart |= 3;
				}
			}
		}

		return std::make_pair(firstPart, secondPart);
	}

	void Board::ConvertUCIMoveToCoreMove(Board::Move& move, Piece& promotedType, const std::string& uciMove) const
	{
		move.index = s_strMoveIndexX.find(uciMove[0]) + 8 * s_strMoveIndexY.find(uciMove[1]);
		move.move = s_strMoveIndexX.find(uciMove[2]) + 8 * s_strMoveIndexY.find(uciMove[3]) - move.index;

		if (uciMove.size() > 4)
			promotedType = (Piece)s_strMoveTypesSmall.find(uciMove[4]);
		else
			promotedType = NONE;
	}

	void Board::ConvertPGNMoveToCoreMove(Board::Move& move, Piece& promotedType, const std::string& pgnMove) const
	{
		int startIndex = pgnMove.find(' ') + 1;

		for (int i = 0; i < pgnMove.size(); i++)
		{
			if (!((pgnMove[i] >= '0' && pgnMove[i] <= '9') || pgnMove[i] == '.' || pgnMove[i] == ' '))
			{
				startIndex = i;
				break;
			}
		}

		Piece typeToMove;
		promotedType = NONE;

		if (pgnMove[startIndex] == '0' || pgnMove[startIndex] == 'O')
		{
			/*Roke*/
			if (pgnMove.find("0-0-0") != std::string::npos || pgnMove.find("O-O-O") != std::string::npos)
				move.move = -2;
			else
				move.move = 2;

			if (m_PlayerToPlay == WHITE)
				move.index = 4;
			else
				move.index = 60;

			return;
		}
		else if (pgnMove[startIndex] >= 'a')
			typeToMove = PAWN;
		else
			typeToMove = (Piece)s_strMoveTypes.find(pgnMove[startIndex]);

		int strMoveDirectionIndex = startIndex;

		while (pgnMove[strMoveDirectionIndex] > '8' || (pgnMove[strMoveDirectionIndex - 1] >= 'A'
			&& pgnMove.size() - (strMoveDirectionIndex + 1) && pgnMove[strMoveDirectionIndex + 1] >= 'a'
			&& typeToMove != PAWN))
		{
			strMoveDirectionIndex += 1;
		}

		int directionIndex = (int)s_strMoveIndexX.find(pgnMove[strMoveDirectionIndex - 1]) + (int)s_strMoveIndexY.find(pgnMove[strMoveDirectionIndex]) * 8;
		if (pgnMove.size() - (strMoveDirectionIndex) > 1 && pgnMove[strMoveDirectionIndex + 1] == '='
			&& s_strMoveTypes.find(pgnMove[strMoveDirectionIndex + 2]) != std::string::npos)
		{
			promotedType = (Piece)s_strMoveTypes.find(pgnMove[strMoveDirectionIndex + 2]);
		}

		//check for the posible pieces 
		std::vector<Board::Move> possibleIndexMoves;
		std::vector<Board::Move> possibleMoves;
		GetAvailableMoves(possibleMoves, typeToMove);

		for (auto& m : possibleMoves)
		{
			if (m.index + m.move == directionIndex)
				possibleIndexMoves.emplace_back(m);
		}

		if (possibleIndexMoves.size() != 1)
		{
			int strHelperIndex = ((typeToMove == PAWN ? 0 : 1) + startIndex);
			int helperIndexX = -1;
			int helperIndexY = -1;

			if (pgnMove[strHelperIndex] >= 'a' && pgnMove[strHelperIndex] <= 'h')
			{
				helperIndexX = (int)s_strMoveIndexX.find(pgnMove[strHelperIndex]);
				if (strMoveDirectionIndex - strHelperIndex > 2 && pgnMove[strHelperIndex + 1] != 'x')
					helperIndexY = (int)s_strMoveIndexY.find(pgnMove[strHelperIndex + 1]);
			}
			else
			{
				helperIndexY = (int)s_strMoveIndexY.find(pgnMove[strHelperIndex]);
				if (strMoveDirectionIndex - strHelperIndex > 2 && pgnMove[strHelperIndex + 1] != 'x')
					helperIndexX = (int)s_strMoveIndexX.find(pgnMove[strHelperIndex + 1]);
			}

			bool xok = false, yok = false;

			for (auto& m : possibleIndexMoves)
			{
				xok = false;
				yok = false;

				if (helperIndexX != -1)
				{
					if (m.index % 8 == helperIndexX)
						xok = true;
					else
						continue;
				}
				else
					xok = true;

				if (helperIndexY != -1)
				{
					if (m.index / 8 == helperIndexY)
						yok = true;
					else
						continue;
				}
				else
					yok = true;

				if (xok && yok)
				{
					move = m;
					return;
				}
			}
		}

		if (possibleIndexMoves.empty())
		{
			//something bad happened
			promotedType = NONE;
			return;
		}

		move = possibleIndexMoves[0];
	}

	void Board::ConvertCLDMoveToCoreMove(Board::Move& move, Piece& promotedType, std::pair<uint8_t, uint8_t> cldMove) const
	{
		Piece typeToMove = NONE;

		uint8_t moveDir = cldMove.first & 0b00111111;
		uint8_t movePartOne = (cldMove.second & 0b11100000) >> 5;
		uint8_t movePartSecond = (cldMove.second & 0b00011100) >> 2;
		uint8_t movePartThird = cldMove.second & 0b00000011;

		int dirX = (moveDir >> 3);
		int dirY = (moveDir & 0b00000111);

		int posX = -1;
		int posY = -1;

		if (movePartOne == 0)
		{
			typeToMove = PAWN;
			
			if ((moveDir >> 3) != movePartSecond)
			{
				posX = movePartSecond;
			}

			if (dirX == 0 || dirY == 7)
			{
				promotedType = (Piece)(movePartThird + 1);
			}
		}
		else if (movePartOne == 1)
		{
			if (movePartSecond == 0 || movePartSecond == 1)
			{
				typeToMove = KING;
			}
			else if (movePartSecond == 2 || movePartSecond == 3)
			{
				typeToMove = (Piece)(movePartThird + 1);
			}
			else if (movePartSecond == 4)
			{
				if (m_PlayerToPlay == WHITE)
					move.index = 4;
				else
					move.index = 60;

				move.move = 2;

				return;
			}
			else if (movePartSecond == 5)
			{
				if (m_PlayerToPlay == WHITE)
					move.index = 4;
				else
					move.index = 60;

				move.move = -2;

				return;
			}
		}
		else if (movePartOne == 2 || movePartOne == 3)
		{
			typeToMove = (Piece)(movePartThird + 1);
			posX = movePartSecond;
		}
		else if (movePartOne == 4 || movePartOne == 5)
		{
			typeToMove = (Piece)(movePartThird + 1);
			posY = movePartSecond;
		}
		else if (movePartOne == 6 || movePartOne == 7)
		{
			static const int converter[] = { -2, -1, 1, 2 };

			if (movePartThird == 0)
			{
				int converted = converter[movePartSecond % 4];
				posX = dirX + converted;

				converted = (std::abs(converted) == 2 ? 1 : 2);

				if (movePartSecond / 4 == 0)
					posY = dirY - converted;
				else
					posY = dirY + converted;

				typeToMove = (Piece)(movePartThird + 1);
			}
			else if (movePartThird == 1 || movePartThird == 2)
			{
				int converted = converter[movePartSecond % 4];
				posX = dirX + converted;

				if (movePartSecond / 4 == 0)
					posY = dirY - converted;
				else
					posY = dirY + converted;

				typeToMove = (Piece)(movePartThird + 1);
			}
			else
			{
				int pos3 = movePartSecond % 4;

				if (pos3 == 0)
				{
					posX = dirX - 3;
					posY = dirY - 3;
				}
				else if (pos3 == 1)
				{
					posX = dirX + 3;
					posY = dirY - 3;
				}
				else if (pos3 == 2)
				{
					posX = dirX - 3;
					posY = dirY + 3;
				}
				else
				{
					posX = dirX + 3;
					posY = dirY + 3;
				}

				if (movePartSecond / 4 == 0)
					typeToMove = BISHOP;
				else
					typeToMove = QUEEN;
			}

			move.index = posX + posY * 8;
			move.move = dirX + dirY * 8 - move.index;

			return;
		}

		int directionIndex = dirX + dirY * 8;

		//check for the posible pieces 
		std::vector<Board::Move> possibleIndexMoves;
		std::vector<Board::Move> possibleMoves;
		GetAvailableMoves(possibleMoves, typeToMove);

		for (auto& m : possibleMoves)
		{
			if (m.index + m.move == directionIndex)
				possibleIndexMoves.emplace_back(m);
		}

		if (possibleIndexMoves.size() != 1)
		{
			bool xok = false, yok = false;

			for (auto& m : possibleIndexMoves)
			{
				xok = false;
				yok = false;

				if (posX != -1)
				{
					if (m.index % 8 == posX)
						xok = true;
					else
						continue;
				}
				else
					xok = true;

				if (posY != -1)
				{
					if (m.index / 8 == posY)
						yok = true;
					else
						continue;
				}
				else
					yok = true;

				if (xok && yok)
				{
					move = m;
					return;
				}
			}
		}

		if (possibleIndexMoves.empty())
		{
			//something bad happened
			promotedType = NONE;
			return;
		}

		move = possibleIndexMoves[0];
	}
}