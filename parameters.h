/* 
     by SCRIER.org 
 
 When this code is useful to your life and the lives of millions of others,
 please send donations through paypal to email "thanks (at) scrier.org".
 
 
 This code's donated copyrightfree by SCRIER to the community in public service.
 This code is provided "as is", with absolutely no assumption of fitness.
 Use at your own risk.  Users assume all liability for usage.
 May be copied, modified, and re-used for  non-monetized  purposes.
Please keep the "Powered by" banner in place; we're a small business that needs funding.

 March 15, 2020
 
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H


// PARAMETERS


// Number of days incubation period, between time a person gets infected
// and the time they start being infectious themselves. 
//
// Note since discrete time ticks in one-day intervals,
// START_INFECTIOUS_AFTER = 0 means starts being infectious the very next day,
// and START_INFECTIOUS_AFTER = 1 means they skip one day so day after tomorrow.
// So people who get infected in the morning cannot infect in the afternoon,
// it's not set up to run that way.
//
// Best estimates are people start getting infectious 2 days or more before they
// start developing mild symptoms, and they start developing symptoms at 3 days.
//
// This number is assumed to be the same, standard, for all cohorts.
#define START_INFECTIOUS_AFTER  1
//#define START_INFECTIOUS_AFTER  0
// "Punch-in Infectious" day.

// At what duration day from the 0'th day they got infected,
// do people STOP being infectious? (Punch-Out Infectious, assuming hospitals).
// If START_INFECTIOUS_AFTER is 0, this is the total # days they're infectious;
// otherwise, you have to subtract START_INFECTIOUS_AFTER to get the
// # total days they're infectious.    .. These are broken down by cohorts.
// For the Milds, this is the last day that they're still sick,
// but wandering around unawares (baseline: assumes no quarantines).
// For the Severes, this is the last day that they're still wandering out sick,
// until they check into the hospital the next day.  If any slots open.
// For the Criticals, this is also the last day that they're out still sick,
// until they check into a hospital & get a ventilator  the next day.
// Milds (including moderate infections) have no need for hospitals to survive.
// Hospitals are assumed to take severes and criticals OUT of infecting others.
// If Severes or Criticals attempt to check into a hospital but are denied,
// they are no longer Unaware, so they continue on the infectious bucket list.
// They never try to go back into the hospital; they simply end up dying.
// 
#define     DAYS_MILD_UNAWARE_INFECTIOUS  21
//#define     DAYS_MILD_UNAWARE_INFECTIOUS  3

//#define   DAYS_SEVERE_UNAWARE_INFECTIOUS  7
// Best current number on this is "10-14 days", maybe 5-9 until notice sick?
// But when check into hospital?  Call it 10.
#define   DAYS_SEVERE_UNAWARE_INFECTIOUS  10

//#define   DAYS_SEVERE_UNAWARE_INFECTIOUS  3
//#define DAYS_CRITICAL_UNAWARE_INFECTIOUS  5
// Maybe very sick gets sick slightly faster than severe?
#define DAYS_CRITICAL_UNAWARE_INFECTIOUS  9
//#define DAYS_CRITICAL_UNAWARE_INFECTIOUS  3
// set two out of three of these to 0 if you only want to check one infect route

// Severes:
// How long does a person tie up a hospital bed, on average,
// if they are Severely sick and likely to die, but NOT critical, needing vent?
// This number is absolutely critical in determining blockage,
// and I don't have any good data on it.
// W.A.G. estimate average of one week.
// Changing this can substantially change outcome, 50M -> 35M.
int DAYS_TO_LIVE_IN_SEVERE_BED = 7;  // INVENTORY TURNS
//int DAYS_TO_LIVE_IN_SEVERE_BED = 3;  // INVENTORY TURNS

// Criticals:
// If going to die, how long is ICU ventilator tied up?
// This number could be longer, as it ignores average attrition.
// But we'll assume best-case, so the vent is freed up rapidly for next person.
#define DAYS_TO_DIE_IN_ICU   7
//#define DAYS_TO_DIE_IN_ICU   3
// If going to live, how long is ICU ventilator tied up?
// this number's pretty solid.
#define DAYS_TO_LIVE_IN_ICU  6*7
// only two simple cases, either die after one week
// or live and get released from hospital after 6 weeks.
// Ignores stochastic cases where people die off between now and then;
// but data is supporting these two rough clusters.
// Assumes no one pulls ventilator away from someone to give it to someone else.
// Assumes doctors do not die off in the middle, and magically always enough
// to staff the hospital beds and the ventilators.



// Suppose you feel sick after a couple days, but get TURNED AWAY at hospital.
// How soon until you die? (100%) But are still infectious and running around...
//#define DAYS_TO_DIE_WITHOUT_SEVERE_BED  0
#define DAYS_TO_DIE_WITHOUT_SEVERE_BED    5
//#define DAYS_TO_DIE_WITHOUT_SEVERE_BED    3

//#define DAYS_TO_DIE_WITHOUT_CRITICAL_ICU  0
//#define DAYS_TO_DIE_WITHOUT_CRITICAL_ICU  2
#define DAYS_TO_DIE_WITHOUT_CRITICAL_ICU  3
// Note:  These people in real life actually infect others at a 
// substantially increased rate.  But no effort is taken to model
// the Beta on these infection rates separately in subcohorts.

// COHORT PERCENTAGES UPON GETTING INFECTED
// Per cent of cases severity.  These will get /100 in initialize()...
double percent_Mild     =  80.0;     // Asymptomatic for a few days, mild case
double percent_Severe   =  15.0;     // Severe case needing hospital; else death
double percent_Critical =   5.0;     // Critical case needing ventilator, or die
// -----------------------100.0% of infected cases.
//
double percent_ICU_die  = 50.0;     // Given using vent, chances of early death
//some say this number is closer to 90%.
//It doesn't matter, as the number of people who are lucky enough to get vents
// will be down in the noise, unless everyone self-isolates.


    // Fudge factor for how many actual cases there are out there,
    // for each reported, official, known case.
    // Even assuming ideal testing,
    // there's good evidence that this lags a number of days behind the curve,
    // so ideally this number would change and eventually saturate down to 1.0
    // as the number of actual infections hits 100% penetration.
    // However, this factor is only really used seriously here,
    // to set up a reasonable initial estimate for the lagged actual counts,
    // and it's also used in reporting to give a ballpark figure
    // of how far behind the curve the authorities are as the days roll by.
    // It does affect how rapidly the hospitals run out of beds.
    // Since if y = x^t, then cy = cx^t, there's no really good way of knowing
    // this until the hospitals start filling up.
    // You can estimate it from (death counts/0.02) / (known infection counts)
    // if you believe the death rate should be 2%, before hospitals swamp.
    // This is the best SWAG, based on death rates in Seattle,
    // and the fact that current test rates as of Wed Mar 18 
    // are...substantially behind the curve.
double Actual_to_official_factor_10 = 10.0;  // Number of actual cases/official.
// !!! INTERESTINGLY, THE VALUE OF THIS FIGURE MAKES LITTLE TO NO DIFFERENCE 
// IN OUTCOME, AND ONLY BUYS US 6 DAYS OF LAG TIME if = 1.0.
// IT SEEMS TO SIMPLY RESET THE CURVE TO LATER, PLUS INITIALIZATION NOISE.
// There is some thought that it could make some difference
// if flirting with hairy edges around hospital saturation points,
// but my personal best guess is at this point that's moot.  Still, perhaps.

// This assumes an average of 8 days of mostly asymptomatic or low behavior
// before a patient actually gets tested positive and declared a case.
// It also assumes 0 days lag in testing->results,
// and no false negatives / false positives.
// Also assumes all infected cases having symptoms get tested, idealized.

// 924,107 Feb '20; ?? 1.3M after tent cities
// Although supposedly US has just under a million beds,
// some other people will get sick too...350,000 births/month to start with.
// MASH hospital cots could double or triple this.
#define BED_COUNT  1000000
//#define BED_COUNT  50

                    // includes 62K delux, 98K basic,
                    // 20K presumed hidden strategic stockpile,
                    // and 20K gratuitous good luck.
                    // It won't matter.
#define VENT_COUNT  200000
//#define VENT_COUNT  50

// SIZE OF U.S. POPULATION,  N does not count children, who are magically immune
// by current thinking.
//
// Note:  Governments are unfortunately lying by omission when they say
// "70%-80% of people will get infected" [so hey, 30%-20% won't!].
// What they neglected to tell you was that
// 21.4% of US population is kids under 14, and
// 32% of population is kids under 24.
// So pretty much all of U.S. adults, who do not shelter in place for months,
// are expected to become infected.  This fits with our baseline.
// Actually, some adults 30 or under will have enough immunity that they don't,
// and some teens, esp. obese or diabetic ones (we have, what, 40% obesity?)
// will in fact catch it, even though they're not supposed to.
// But this is the best SWAG.
//
// Then we set it to a nice, round number, so it's easy to see the effects.
// Modify this in place if you don't like it; the convenience of parameter file.

//double N_USpop_total_count = 329338025 * 0.786;  //330M as of Mar 13, 2020
// that's 258,859,688 adults in play.
// I'm assuming N should be adult:adult contacts, and not the larger population.
// Here we also magically assume children do not offer any transmission (unsub.)
//double N_USpop_total_count = 260000000;  //number of adults over 14
double N_USpop_total_count = 250000000;  //number of adults over 14
// but everyone 14 and under gets a free pass.


// Used in the S.I.R. model.
// Incremental new actual infections (delta I) =
//      Beta * ( S * I ) / N.
// Of course.  Sigh.  And you thought this was reality.
#define DEFAULT_BETA    0.42
double Beta_transmission_rate_per_day = DEFAULT_BETA; //unclear why this is off.
//double Beta_transmission_rate_per_day = 0.32;  // 2.3x in 3 days for US [WHO]
            // That's 10x in 8.3 days, call it 8, @ Mar 15 '20

#endif /* PARAMETERS_H */