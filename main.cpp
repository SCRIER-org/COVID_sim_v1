/* 
 * File:   main.cpp
 * Author: SCRIER.org
 
 When this code is useful to your life and the lives of millions of others,
 please send donations through paypal to email "thanks (at) scrier.org",
or  gofundme.com/f/scrier   .
 
 
 This code's donated copyrightfree by SCRIER to the community in public service.
 This code is provided "as is", with absolutely no assumption of fitness.
 Use at your own risk.  Users assume all liability for usage.
 May be copied, modified, and re-used for non-monetized purposes.
Please keep the Powered By banner in place; we're a small business that needs funding.
 
 * Created on March 15, 2020, 3:32 PM
 
--Model parameters in "parameters.h" for most constants.
 --Basic model uses bucket brigades to simulate delays accurately
   but uses hard averages and deterministic math instead of decays/stochastics
   to simulate occurrences.  Had to get something up and running fast.
 
 --Assumes ppl who are "severely ill" will by definition die if not hospitalizd.
 --Assumes people in hospitals are no longer infectious to community at large
 --People who get out of hospitals may be infectious for another week or so,
  but this population never got modeled, so we're ignoring that for this version
 --Dead people are not infectious
 --Mild people get better by themselves after say 21 days but are infectious
 --Servere hospital stays get better after say 7 days, then all live
 --Magically no one dies who is only Severe, but makes it into a hospital.
 --Half of criticals tie up ventilator then rudely die after say 1 week
 --Half of critical ICU stay w/ventilator get better after say 6 weeks
 --Well people check out of the hospital in the morning
 --Available beds and vents are counted at noon
 --They hand out available vents/beds in the afternoon, but ppl die in morning,
   it only takes a few hrs to clean used vents and beds to hand out in afternoon
 --Severe or critical people who don't get into a hospital/ICU 
   still take say a couple days to die.
 --Severe or critical people who get denied hospitalization go home to sulk
   then die in a few days, and never get back into line, because the line
   will almost certainly already be full; too sick; and they'll be passed over.
   Last in, First out, & only people who start feeling sick on THAT DAY win. 
 -- assumes people can only get infected once.  Ignores virus mutations.
-- Assumes no one pulls ventilator away from someone to give it to someone else.
-- Assumes doctors do not die off in the middle, and magically always enough
   to staff the hospital beds and the ventilators.
-- Assumes severely and critically infected people who get turned away
  at the hospital are still infectious at the same rate as others,
  until they die.  Real life says they're substantially more infectious.
  THIS WILL IMPACT REAL LIFE AND MAKE HOSPITAL DENIAL MORE DEADLY THAN MODELED.
 -- Assumes children are magically immune.  And do not act as carriers.
 -- Ignores transmission vector between parents-->children (confirmed)
    children -> children (unconfirmed), and children -> parents (unconfirmed)
    since children are all magically immune.  And not counted as part of N.
 In real life, children seem to get the virus but be Mild, mostly asymptomatic.
 
 
 
Overview:
This rough code was slapped together in about a day and a half to get results.
It's made very basic and crude, with only one .cpp and one .h, on purpose,
so that anyone can easily see what it's doing and modify it themselves.
 There are probably errors somewhere as it is written in haste.
 At the very least the baseline Beta seems to be off, it should be 0.23 but
 I had to crank it up to 0.4 in order to make the numbers fit properly.
 And the most recent data indicates even this looks low for Wed Mar 18th.
So please send bug fix suggestions to the github page.

The main contribution is a rough simulation of hospital beds and ventilators
running out of supply, and the effects on the population.
Since even Goldman Sachs is estimating only 3M U.S. deaths, as of March 16th'20,
I'm drawing attention to what I believe to be vitally critical results.
My baseline, without severe isolation practices, shows roughly 25-50M deaths.
 
 
 
 The population gets divided into children
 
Uses a discrete-time deterministic simulation, with clock tick of 1 day.
I divide sick people into three cohorts:
Mild infections (including Moderate), Severe, and Critical infections.
People are assigned into a cohort the day they get infected,
but they don't know it yet.  (% params).
For, say, a day of incubation they're not infectious themselves (param). 
They continue unaware that they have it, but infectious,
(implemented as a)


 */

#include <stdio.h>
#include <locale.h>     // for commas.
#define ENABLE_COMMAS()     setlocale(LC_NUMERIC, "")
#include "parameters.h"     // ours.

// Size of bucket brigade delay arrays, used internally for array space inits.
// must add 1 because 0-based, otherwise math will fall off the edge
#define DAYS    DAYS_TO_LIVE_IN_ICU + 1

int DontHaveHospitalBeds   ;     // flag
int DontHaveICUVentilators ;     // flag

// STATE VARIABLES:
// rough S I R infection model.  Susceptibles. Infecteds. Recovereds, incl dead
// Actual pop of U.S., not counting children, who're time T NOT infected at all:
int S_actual_uninfected_total_count;
   
// 
int I_actual_daily_infections_rolling_netcounts_per_day[DAYS];

// Three cohorts of infectious people, mostly not aware of their infections yet
int     IMild_unrecognized_infectious_rolling_netcounts_per_day[DAYS];
int   ISevere_unrecognized_infectious_rolling_netcounts_per_day[DAYS];
int ICritical_unrecognized_infectious_rolling_netcounts_per_day[DAYS];

// I don't think this ever got wired in.
// These are people who have recovered from a severe infection
// but still have a residual infection and are still infectious.
// Papers note these exist, but it's a second-level factor. Left for round 2.
//int ISevere_recovered_infectious_rolling_netcounts_per_day[DAYS]; // starts @ 0

// Estimated but official, known total U.S. infections count, cumulative
// (so includes recovereds / deads, I believe).
int Iest_official_infections_total_count;
//int Iest_official_infections_rolling_netcounts_per_day[30];


int Rdead_total_count;
int Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[DAYS];

int Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[DAYS];


int Rrecovered_noninfectious_total_count;


// This is a sink total tally that ignores the number of cured or died.
int I_actual_infections_total_count; 
//  This is the number actually used for computations.
//  It is recomputed from raw bucket lists every day, so no subtractions.
int I_total_todays_infectious;
int Inew_count_todays_actual_new_infections;
int Inew_count_todays_Mild_infections;
int Inew_count_todays_Severe_infections;
int Inew_count_todays_Critical_infections;







int US_HospitalBed_count;  //924107;  // after tent cities
int US_Ventilator_count; // includes 62K delux, 98K basic,
                                    // 20K presumed hidden strategic stockpile,
                                    // and 20K gratuitous good luck.
                                    // It won't matter.
int Total_Beds_Used;
int Total_Ventilators_Used;

int Beds_Remaining;
int Ventilators_Remaining;

int Want_Beds;
int Get_Beds;
int Dont_Get_Beds;

int Want_Vent;
int Get_Vent;
int Dont_Get_Vent;

int Total_Dont_Get_Beds;
int Total_Dont_Get_Vent;
int Total_No_Bed_Deaths;
int Total_No_Vent_Deaths;
int Total_InHosp_Deaths;
int OtherDeaths;

////////////////////////////utilities//////////////////////////////////////////
char date[7];
char * T_to_dateName(int T) // returns printstring of date, given time t
{
    T += 13;    // Time 0 is Mar 13, 2020
    if(T<=31) { sprintf(date, "Mar %2d", T); return(date); }  T-= 31;
    if(T<=30) { sprintf(date, "Apr %2d", T); return(date); }  T-= 30;
    if(T<=31) { sprintf(date, "May %2d", T); return(date); }  T-= 31;
    if(T<=30) { sprintf(date, "Jun %2d", T); return(date); }  T-= 30;
    if(T<=31) { sprintf(date, "Jul %2d", T); return(date); }  T-= 31;
    if(T<=31) { sprintf(date, "Aug %2d", T); return(date); }  T-= 31;
    if(T<=30) { sprintf(date, "Sep %2d", T); return(date); }  T-= 30;
    if(T<=31) { sprintf(date, "Oct %2d", T); return(date); }  T-= 31;
    if(T<=30) { sprintf(date, "Nov %2d", T); return(date); }  T-= 30;
    if(T<=31) { sprintf(date, "Dec %2d", T); return(date); }  T-= 31;
    if(T<=31) { sprintf(date, "Jan'21 %2d", T); return(date); }  T-= 31;
    if(T<=28) { sprintf(date, "Feb'21 %2d", T); return(date); }  T-= 28;
    if(T<=31) { sprintf(date, "Mar'21 %2d", T); return(date); }  T-= 31;
    sprintf(date, "2021");  return(date);
}


///////////////////////////////////////////////////////////////////////////////
void initialize()   // Set up world at Mar 13, 2020.
// This is called to reset every run.
{
    int d;
    
    // Use round numbers for ease of understanding.
    US_HospitalBed_count = BED_COUNT;//924,107 Feb '20; ?? 1.3M after tent cities
    US_Ventilator_count  = VENT_COUNT;// includes 62K delux, 98K basic,
                                    // 20K presumed hidden strategic stockpile,
                                    // and 20K gratuitous good luck.
                                    // It won't matter.
    
    DontHaveHospitalBeds   = 0;     
    DontHaveICUVentilators = 0; 
    Total_Dont_Get_Beds  = 0;
    Total_Dont_Get_Vent  = 0;
    Total_No_Bed_Deaths  = 0;
    Total_No_Vent_Deaths = 0;
    Total_InHosp_Deaths  = 0;
    OtherDeaths = 0;

    Iest_official_infections_total_count = 1678;  // Mar 13 ?.
    I_actual_infections_total_count = 0;  // init
    I_total_todays_infectious = 0;  // init.

    Rdead_total_count = 0; //57;
    Rrecovered_noninfectious_total_count = 0;  // arbitrary
    
    Rrecovered_noninfectious_total_count = 0;    // arbitrary
    
    // clear out everything:
    for(d=0; d<DAYS; d++)
    {
        IMild_unrecognized_infectious_rolling_netcounts_per_day[ d ] = 0;
        ISevere_unrecognized_infectious_rolling_netcounts_per_day[ d ] = 0;
        ICritical_unrecognized_infectious_rolling_netcounts_per_day[ d ] = 0;
        

        Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d]  = 0;  
        Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d]  = 0;

        //ISevere_recovered_infectious_rolling_netcounts_per_day[ d ] = 0; //arb
        
        I_actual_daily_infections_rolling_netcounts_per_day[  d ] = 0;  // init
    }
    
    
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
    double A = Actual_to_official_factor_10;
    
    // d goes backwards into time, as it's rolling. d = Number of days before t.
    I_actual_infections_total_count +=   // figures are from WHO, but prob low
    I_actual_daily_infections_rolling_netcounts_per_day[ 0 ] =  A*(1678 - 1264); 
    I_actual_infections_total_count +=
    I_actual_daily_infections_rolling_netcounts_per_day[ 1 ] =  A *(1264 - 987); 
    I_actual_infections_total_count +=
    I_actual_daily_infections_rolling_netcounts_per_day[ 2 ] =  A * (987 - 696); 
    I_actual_infections_total_count +=
    I_actual_daily_infections_rolling_netcounts_per_day[ 3 ] =  A * (696 - 472); 
    I_actual_infections_total_count +=
    I_actual_daily_infections_rolling_netcounts_per_day[ 4 ] =  A * (472 - 213); 
    I_actual_infections_total_count +=                          // sic
    I_actual_daily_infections_rolling_netcounts_per_day[ 5 ] =  A * (213 - 173); 
    I_actual_infections_total_count +=                          // sic
    I_actual_daily_infections_rolling_netcounts_per_day[ 6 ] =  A * (173 - 133); 
    I_actual_infections_total_count +=                          
    I_actual_daily_infections_rolling_netcounts_per_day[ 7 ] =  A * (133 - 100); 
    I_actual_infections_total_count +=                          
    I_actual_daily_infections_rolling_netcounts_per_day[ 8 ] =  A * (100); 
    // everybody added on day 8, close enough
    

    for(d=0; d<=8; d++)
    {

      ISevere_unrecognized_infectious_rolling_netcounts_per_day[ d ] =
        percent_Severe*I_actual_daily_infections_rolling_netcounts_per_day[ d ];
      
      // Arbitrarily assign half of the initial criticals to inside ICU,
      // and half to Don't know it yet, but will in a few days.
      // This is down in the noise.
ICritical_unrecognized_infectious_rolling_netcounts_per_day[ d ] =
 0.5 *  percent_Critical*I_actual_daily_infections_rolling_netcounts_per_day[d];
Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d] =
   0.5* percent_Critical*I_actual_daily_infections_rolling_netcounts_per_day[d];
      
       IMild_unrecognized_infectious_rolling_netcounts_per_day[ d ] =
         I_actual_daily_infections_rolling_netcounts_per_day[ d ] 
            - ISevere_unrecognized_infectious_rolling_netcounts_per_day[ d ]
            - ICritical_unrecognized_infectious_rolling_netcounts_per_day[ d ]
            - Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d]   
               ;
               
    }
    
    S_actual_uninfected_total_count = N_USpop_total_count 
                                      - I_actual_infections_total_count;
    

    
}

///////////////////////////////////////////////////////////////////////////////


void clock_one_day()
{   int d;
    int deaths;

    

 // COMPUTE TOTAL INFECTIOUS (outside hospitals) FOR TODAY

    // Tally:  How many INFECTIOUS people do we have??
    I_total_todays_infectious = 0;  // init.
    
    // Add in all the Mild cohort buckets.
    for(d=START_INFECTIOUS_AFTER; d<DAYS_MILD_UNAWARE_INFECTIOUS; d++)
    {
       I_total_todays_infectious +=  
               IMild_unrecognized_infectious_rolling_netcounts_per_day[d]; 
    }
    
    // Add in all the Severe, infectious, not in the hospital  buckets.
    // we'll check into hosp and stomp buckets to 0, except if hosp full.
    for(d=START_INFECTIOUS_AFTER; 
            d<DAYS_SEVERE_UNAWARE_INFECTIOUS + DAYS_TO_DIE_WITHOUT_SEVERE_BED;
            d++)
    {
       I_total_todays_infectious +=  
               ISevere_unrecognized_infectious_rolling_netcounts_per_day[d]; 
    }
    
    // Add in all the Critical, infectious, not in the hospital  buckets.
    // we'll check into hosp and stomp buckets 0, except if hosp full.    
    for(d=START_INFECTIOUS_AFTER; 
          d<DAYS_CRITICAL_UNAWARE_INFECTIOUS + DAYS_TO_DIE_WITHOUT_CRITICAL_ICU; 
          d++)
    {
       I_total_todays_infectious +=  
               ICritical_unrecognized_infectious_rolling_netcounts_per_day[d]; 
    }    
    
    // Debugging: Set this to an easy number to check what happens downstream.
    //I_total_todays_infectious = 1000;
    
//    printf("I_total_todays_infectious  = %'d\n", I_total_todays_infectious);    
    
    
    
 // DEATHS *OUTSIDE* HOSPITAL (from prev turned aways, delayed through buckets)
    
    // If didn't get to hospital, at this point all Severes die.  
    // ...originally implemented as a point event,
    // but requires a loop to stomp crappy initializations who don't die.
    // This results in a few seconds of wasted time per run.
    // Take the loop back out if you're running on a micro computer.
    // sigh.  loop has to run backwards to get proper debugging printout.
    //  ...Modify this into a rolling poisson for Version 2.
    for(    d = DAYS - 1;
            d>=DAYS_SEVERE_UNAWARE_INFECTIOUS + DAYS_TO_DIE_WITHOUT_SEVERE_BED; 
            d--)
    {
    deaths = ISevere_unrecognized_infectious_rolling_netcounts_per_day[d];
    Total_No_Bed_Deaths += deaths;      // incr count of severes denied beds
    Rdead_total_count   += deaths;      // incr total deaths count
    
    // People who die because they couldn't get to hosp can't infect any more
    ISevere_unrecognized_infectious_rolling_netcounts_per_day[d] = 0;
    }
    
    /** /
    if(deaths != ISevere_unrecognized_infectious_rolling_netcounts_per_day[d+1])
       printf("New Old_Sever_nobedDeaths: %'d  + %'d inits\t  =>  tot %'d.\n", 
                deaths, 
               (Total_No_Bed_Deaths - deaths),
               Total_No_Bed_Deaths);
    else
    //if(deaths) 
        printf("New Old_Sever_nobedDeaths: %'d \t  =>  tot %'d.\n", 
                deaths, Total_No_Bed_Deaths);
       /**/

    
    for( d = DAYS - 1;
         d>=DAYS_CRITICAL_UNAWARE_INFECTIOUS + DAYS_TO_DIE_WITHOUT_CRITICAL_ICU; 
         d--)
    {
    // If didn't get to hospital, at this point all Criticals die.  .
    //  modify this into a rolling poisson next time around.
    deaths =  ICritical_unrecognized_infectious_rolling_netcounts_per_day[d];   
    Total_No_Vent_Deaths += deaths;      // incr count of severes denied beds
    Rdead_total_count    += deaths;      // incr total deaths count
    
    
       //
    // People who die because they couldn't get to hosp can't infect any more
    ICritical_unrecognized_infectious_rolling_netcounts_per_day[d] = 0;

    }
    
    /** /
    if(deaths != ISevere_unrecognized_infectious_rolling_netcounts_per_day[d+1])
       printf("New Old_Crit_NoVentDeaths: %'d  + %'d inits\t  =>  tot %'d.\n", 
                deaths, 
               (Total_No_Vent_Deaths - deaths),
               Total_No_Vent_Deaths);
    else        
    //if(deaths) 
        printf("New Old_Crit_NoVentDeaths: %'d \t  =>  tot %'d.\n", 
                deaths, Total_No_Vent_Deaths);
      /**/ 


// MORNING:  HOSPITAL CHECKOUTS, IN THE MORNING
        
    // Severe people get well in hospital after x days, graduate.
    // Ideally this would also include being quietly infectious in the
    // community for another couple of days; left as exercise for the reader
    //  ...because LIVE is 1's based count, but array is [0] based, this works.
    for(d=DAYS_TO_LIVE_IN_SEVERE_BED; d<DAYS; d++)
    {
     // the loop may be overkill.  But just in case someone checks back in.
        
        Rrecovered_noninfectious_total_count +=  
              // This term "should" always be 0:
              Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d];
        // aaand, they check out, and are no longer taking up that bed space.
        Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d] = 0;
        // this again is almost all redundant and complete overkill.
    }
    
    // People who are still in ventilators and haven't died after a week,
    // get to graduate and recover after 6 weeks.
    for(d=DAYS_TO_LIVE_IN_ICU; d<DAYS; d++)
    {
     // the loop may be overkill.  But just in case someone checks back in.
        
        Rrecovered_noninfectious_total_count +=  
              // This term "should" always be 0:
              Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d];
        // aaand, they check out, and are no longer taking up that ventilator.
        Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d] = 0;
        // this again is almost all redundant and complete overkill.
    }

        
    // MORNING:  DEATHS *INSIDE* HOSPITAL:
    //
    // CHECKING OUT OF HOSPITAL BY DYING  ...This happens in the Morning.
                
    // Magically no one dies who is only Severe, but makes it into a hospital.
                
    //  People don't die if they graduate from Severe Bed successfully.
                
                
    // As for Criticals in the hospital,
    // this should be another poisson, but it's a point event (average) here.
    // Tally up ICU deaths at 7 days in:
    deaths = Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[ 
            DAYS_TO_DIE_IN_ICU ] * percent_ICU_die;
    Total_InHosp_Deaths += deaths;      // incr count of severes denied beds
    Rdead_total_count    += deaths;      // incr total deaths count 
    //if(deaths) 
    
    /** /
     printf("New InHosp_Criticl_Deaths: %'d   =   %'d * %.2f  =>  tot %'d.\n", 
             deaths,
  Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[ 
            DAYS_TO_DIE_IN_ICU ],            
             percent_ICU_die,
             Total_InHosp_Deaths
             );
    /**/
    // If they're going to die, they all die at 7 days in ICU. Here's who's left
    Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[ 
            DAYS_TO_DIE_IN_ICU ] -= deaths;
            
            
    //        *= (1.0 - percent_ICU_die);

 
    
        
      
    
    
    
        
        
// NOONTIME:  HOW MANY BEDS AND VENTS ARE AVAILABLE???
        
    // Compute number of severe Beds in use, and number of Ventilators in use.
    // This is AFTER folks check out in the morning.
    Total_Beds_Used = 0;
    for(d=0; d<DAYS_TO_LIVE_IN_SEVERE_BED; d++)
        Total_Beds_Used += 
               Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d];
    
    Total_Ventilators_Used = 0;    
    for(d=0; d<DAYS_TO_LIVE_IN_ICU; d++)
        Total_Ventilators_Used +=
              Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d];
    
    // we now have Total_Beds_Used and Total_Ventilators_Used for this morning,
    // after checkouts.
    
    Beds_Remaining        = US_HospitalBed_count - Total_Beds_Used;
    Ventilators_Remaining = US_Ventilator_count  - Total_Ventilators_Used;
    
    /** /
    if(Beds_Remaining < 0) 
    {
     printf("\n\n\n***ERROR: BedsRemaining should never be %d!!***\n\n", 
                Beds_Remaining);
    }
    if(Ventilators_Remaining < 0)
    {
     printf("\n\n\n***ERROR: Ventilators_Remaining should never be %d!!***\n\n", 
                Ventilators_Remaining);  
    }
    printf("Beds : Used: %'9d   +  left: %'9d  = Supply %'9d\n", 
            Total_Beds_Used, Beds_Remaining, US_HospitalBed_count);
    printf("Vents: Used: %'9d   +  left: %'9d  = Supply %'9d\n", 
            Total_Ventilators_Used, Ventilators_Remaining, US_Ventilator_count);
    /**/
    
// NOON:  HOW MANY NEW PEOPLE GET INFECTED TODAY??
    
    // SIR model.  New infections proportional to product of
    // actual uninfected population and actual infectious folks,
    // with a Beta term for the incremental spread.
    // These are all newly generated infections.
    // We'll take out the "got well"s later.
    
    Inew_count_todays_actual_new_infections = 0.5 + // for rounding
            Beta_transmission_rate_per_day  *
            // oops integer overflow if we don't use doubles here...
            (  (double) S_actual_uninfected_total_count * 
               (double) I_total_todays_infectious
            ) / N_USpop_total_count;
    
    // This is a sink total that ignores the number of cured or died.
    I_actual_infections_total_count += Inew_count_todays_actual_new_infections;
    
    // Virgin population drops by number that just got infected today.
    // assumes people can only get infected once.
    S_actual_uninfected_total_count -= Inew_count_todays_actual_new_infections;
    if(S_actual_uninfected_total_count < 0)
    {
        printf("GAME OVER--Entire Country's Adults Infected\n\n\n\n");
        S_actual_uninfected_total_count = 0;
    }
    
    // Official (estimated) count is around 1/10 of the actual count,
    // due to 
    // (1) overly-strict bureaucracy red tape on testing permissions
    // (2) test kits' profoundly poor distribution chains
    // (3) turn-around delays in testing
    // (4) ceiling effects as number of test lab processing slots saturates.
    // These should actually converge to the final actual count,
    // but because this is only a feedback number, not used in calculations,
    // it doesn't make any difference in either case anyway.
    // Real-life tests do not make much difference either
    // now that we've ensured homogeneous mixing and hospital overwhelm.
    Iest_official_infections_total_count += 
                    Inew_count_todays_actual_new_infections / 
                                                  Actual_to_official_factor_10;
    
    /** /
    printf("Inew_count_todays_actual_new_infections  = %'d, ESTIMATE %'d\n", 
                Inew_count_todays_actual_new_infections,
                Iest_official_infections_total_count);
    /**/
    
    // DISTRIBUTE THE NEW INFECTIONS AMONGST THE THREE COHORTS
    
    Inew_count_todays_Severe_infections = 
            percent_Severe   * Inew_count_todays_actual_new_infections;
    Inew_count_todays_Critical_infections =
            percent_Critical * Inew_count_todays_actual_new_infections;
    
    Inew_count_todays_Mild_infections =  
            Inew_count_todays_actual_new_infections
            -  Inew_count_todays_Severe_infections
            - Inew_count_todays_Critical_infections  ;   // integers. Need 100%.
                
    
        

    
// BOOKKEEPING, AT NOONTIME.  SHIFT ALL THE OLD BUCKETS BACK.    

    // shift running buckets one day back.  Work from the back.
    for(d=DAYS-1; d>=1; d--)
    {

        
        Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d] =
        Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d-1];
        
        Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d] =
        Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d-1];
        
        IMild_unrecognized_infectious_rolling_netcounts_per_day[d] =      
        IMild_unrecognized_infectious_rolling_netcounts_per_day[d-1];
        ISevere_unrecognized_infectious_rolling_netcounts_per_day[d] =      
        ISevere_unrecognized_infectious_rolling_netcounts_per_day[d-1];        
        ICritical_unrecognized_infectious_rolling_netcounts_per_day[d] =      
        ICritical_unrecognized_infectious_rolling_netcounts_per_day[d-1];  
        
        //ISevere_recovered_infectious_rolling_netcounts_per_day[d] =      
        //ISevere_recovered_infectious_rolling_netcounts_per_day[d-1];
        
        I_actual_daily_infections_rolling_netcounts_per_day[d] =
        I_actual_daily_infections_rolling_netcounts_per_day[d-1];       
      
        
        
    }
    
    
// FILL IN THE NEW BUCKET IN EACH LINE, IN THE FRONT
    
    // Daily Actual Infections:
    // Fill in the front bucket from new figures:
    I_actual_daily_infections_rolling_netcounts_per_day[0] =
            Inew_count_todays_actual_new_infections;
    
    IMild_unrecognized_infectious_rolling_netcounts_per_day[0] = 
            Inew_count_todays_Mild_infections;
    ISevere_unrecognized_infectious_rolling_netcounts_per_day[d] =      
            Inew_count_todays_Severe_infections;        
    ICritical_unrecognized_infectious_rolling_netcounts_per_day[d] =      
            Inew_count_todays_Critical_infections; 


    // PEOPLE STANDING IN LINE TO CHECK IN.  HOW MANY GET IN??
    // COMPUTE AVAILABLE RESOURCES:
    
    Want_Beds = ISevere_unrecognized_infectious_rolling_netcounts_per_day[ 
                                          DAYS_SEVERE_UNAWARE_INFECTIOUS ];
    if( Beds_Remaining - Want_Beds >= 0)
    {
        Get_Beds = Want_Beds;
    }
    else if( Beds_Remaining > 0 )
    {
        Get_Beds = Beds_Remaining;     
    }
    else  // Beds_Remaining == 0 (or less)
    {
        Get_Beds = 0;
    }
    Dont_Get_Beds =  Want_Beds - Get_Beds;  // works for all 3 cases.
    
    /** /
    printf("Beds: Want[%d]: %'d   Avail: %'d   Get: %'d   Don't: %'d\n",
            DAYS_SEVERE_UNAWARE_INFECTIOUS,
            Want_Beds, Beds_Remaining, Get_Beds, Dont_Get_Beds
            );
    /**/

    Want_Vent = ICritical_unrecognized_infectious_rolling_netcounts_per_day[ 
                                         DAYS_CRITICAL_UNAWARE_INFECTIOUS ];
    
    if( Ventilators_Remaining - Want_Vent >= 0)
    {
        Get_Vent = Want_Vent;
    }
    else if( Ventilators_Remaining > 0 )
    {
        Get_Vent = Ventilators_Remaining;     
    }
    else  // Vent_Remaining == 0 (or less)
    {
        Get_Vent = 0;
    }
    Dont_Get_Vent =  Want_Vent - Get_Vent;  // works for all 3 cases.    

    /** /
    printf("Vnts: Want[%d]: %'d   Avail: %'d   Get: %'d   Don't: %'d\n",
            DAYS_CRITICAL_UNAWARE_INFECTIOUS,
            Want_Vent, Ventilators_Remaining, Get_Vent, Dont_Get_Vent
            );    
    /**/
    
        // Update beds occupied etc.
    // We already ran a running total of these at noontime, before new checkins
    // But now we have to add all the new folks who check in  in the afternoon.
    Total_Beds_Used += Get_Beds;
    Total_Ventilators_Used += Get_Vent;
    
    
    if( Dont_Get_Beds > 0 )
    {
         DontHaveHospitalBeds++;     // someone is going begging, run out.
         if( DontHaveHospitalBeds == 1)
             printf("\nRAN OUT OF HOSPITAL BEDS\n");
    }
    else 
    {   if(DontHaveHospitalBeds) printf("\nHOSPITAL BEDS FINALLY AVAILABLE\n");
        DontHaveHospitalBeds = 0;
    }
    
    if( Dont_Get_Vent > 0 )
    {
         DontHaveICUVentilators++;     // someone is going begging, run out.
                  if( DontHaveICUVentilators == 1)
             printf("\nRAN OUT OF VENTILATORS\n");
    }
    else
    {
        if(DontHaveICUVentilators) printf("\nVENTILATORS FINALLY AVAILABLE\n");
        DontHaveICUVentilators = 0; 
    }
    
    
    
    // Note this is all after we've rolled.  
    // This works because DAYS is 1-based, but unrec[] are 0-based.
    
    
// AFTERNOON: CHECKING IN TO HOSPITAL, NOW THAT THERE'S SPACE:

    // Reset clock when a Severe is hospitalized.
    // Severes check into hospital around day 5, cease infecting ppl inside hosp
    Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[0] =
         Get_Beds;
    
    // People turned away outside hospital are still infectious:
    ISevere_unrecognized_infectious_rolling_netcounts_per_day[ 
                                          DAYS_SEVERE_UNAWARE_INFECTIOUS ] 
            = Dont_Get_Beds;
    // Remaining severe people who don't check in, are still infectious.

    

    // Reset his internal clock when a Critical is hospitalized.
    // Criticals check into hospital around day 4, cease infecting inside hosp.
    // But only the number of people who have a ventilator available get in:
    Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[0] =
            Get_Vent;
    
    // That day for the rolling bucket brigade subtracts the number of people
    // who check in, and gets reset to the number of people left who CAN'T:
                ICritical_unrecognized_infectious_rolling_netcounts_per_day[ 
                                         DAYS_CRITICAL_UNAWARE_INFECTIOUS ]
                         = Dont_Get_Vent;
                // If vents are full, we go begging.
    // Note: remaining critical people who don't check in, are still infectious.
                
                
    
    
    
    
    
     Total_Dont_Get_Beds += Dont_Get_Beds;
     Total_Dont_Get_Vent += Dont_Get_Vent;
     
     
     
     /** /
         printf("Actual new infections:\n");
    for(d=0; d<12; d++)
    {
        printf("%'5d ", I_actual_daily_infections_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("Mild unrecognized infections:\n");
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              IMild_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n"); 
    printf("Severe unrecognized infections:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              ISevere_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("Critical unrecognized infections:\n");     
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              ICritical_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    
        printf("Hospitalized Severe noninfectious:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    
        printf("Hospitalized Critical noninfectious:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("\n");    
    /**/

     
     
     
}

//////////////////////////////////////////////////////////////////////////////

void    // Entire One-year pass over a whole scenario.  See "clock" for dailies.
do_one_run( char * scenario_name,
        double StartingBeta,
        int Time_of_NewLockdown_in_days_since_Mar13,
        double LockdownNewBeta,
        int Until_T
        )
{   int T, d;  //Absolute discrete time (in days); relative index counter (days)

    printf("\n\n\n\nStarting Run:\n");
    printf("%s\n", scenario_name);
    
    Beta_transmission_rate_per_day = StartingBeta; 
    //printf("Starting on %s:\n",  t_to_date(0) );
    initialize();
    
    /** /
    printf("Actual new infections:\n");
    for(d=0; d<12; d++)
    {
        printf("%'5d ", I_actual_daily_infections_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("Mild unrecognized infections:\n");
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              IMild_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n"); 
    printf("Severe unrecognized infections:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              ISevere_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("Critical unrecognized infections:\n");     
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              ICritical_unrecognized_infectious_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    
        printf("Hospitalized Severe noninfectious:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              Rhospitalized_noninfectious_severe_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    
        printf("Hospitalized Critical noninfectious:\n");    
    for(d=0; d<12; d++)
    {
        printf("%'5d ", 
              Rhospitalized_noninfectious_critical_rolling_netcounts_per_day[d] );
    }
    printf("\n");
    printf("\n");    
    
     printf("AT KICKOFF:\n");
            
    printf(
  "Total  NoBedDeaths: %'d, Total NoVentDeaths: %'d,  Total InHosp Deaths: %'d  => Total Deaths %d\n", 
            Total_No_Bed_Deaths, Total_No_Vent_Deaths, Total_InHosp_Deaths,
            Rdead_total_count);
/**/
    
    // Discrete time, currently in loop from Mar 13 through Dec 31, 2020.
    for(T=0; T<Until_T; T++)    // time in days since Mar 13, 2020
    {
        // You get One lockdown reset in this setup,
        // add more if you wish.
        // Select the date that the lockdown happens.
        // This is a step-function trigger
        // that resets this global variable only when it goes off.
        if(T == Time_of_NewLockdown_in_days_since_Mar13)   
            Beta_transmission_rate_per_day = LockdownNewBeta;
        // this is a reset, it continues on at new value for rest of this run.
        
  /** /      
printf(
 "\n\n[%d] %s:   actual: %'d,    official: %'d,   infectious: %'d,  @dead: %'d (%'d + %'d + %'d) \n", 
                T, T_to_dateName(T), 
                I_actual_infections_total_count, 
                Iest_official_infections_total_count,
                I_total_todays_infectious,
                Rdead_total_count,
        Total_No_Bed_Deaths,
        Total_No_Vent_Deaths,
        Total_InHosp_Deaths
                );        
        /**/
        
        clock_one_day();
        
        
        
printf(
// "[%d] %s:   actual: %'d,    official: %'d,   infectious: %'d,  @dead: %'d (%'d + %'d + %'d) \n", 
 "[%d] %s:   actual: %'d,    official: %'d,   infectious: %'d,   dead: %'d \n", 
                T, T_to_dateName(T), 
                I_actual_infections_total_count, 
                Iest_official_infections_total_count,
                I_total_todays_infectious,
                Rdead_total_count
        //Total_No_Bed_Deaths,
        //Total_No_Vent_Deaths,
        //Total_InHosp_Deaths
                );
        
/** /
        printf("  beds used: %'9d,        vents used: %'9d\n", 
                Total_Beds_Used, Total_Ventilators_Used);
        
        printf("beds denied: %'9d,      vents denied: %'9d\n", 
                Dont_Get_Beds, Dont_Get_Vent);
        
        printf("Total beds denied: %'9d,      Total vents denied: %'9d\n",         
        Total_Dont_Get_Beds, Total_Dont_Get_Vent);
        /**/
        
        /** /
        printf(" S: %'d  I: %'d  New: %'d (%'d / %'d / %'d)\n", 
                S_actual_uninfected_total_count,
                I_total_todays_infectious,
                Inew_count_todays_actual_new_infections,
                Inew_count_todays_Mild_infections,
                Inew_count_todays_Severe_infections,
                Inew_count_todays_Critical_infections
                );
        /**/
               
        
            printf("\n"); 
        
    }
   
    
    
    
    // Unless transients have died out, there's still some in pipeline next wk:
    int SoonBedDeaths = Total_Dont_Get_Beds - Total_No_Bed_Deaths;
    if(SoonBedDeaths < 0) SoonBedDeaths = 0;
    // these numbers are off because of initialization noise.
    
    
    
    // Unless transients have died out, there's still some in pipeline next wk:
    int SoonVentDeaths = Total_Dont_Get_Vent - Total_No_Vent_Deaths;
    if(SoonVentDeaths < 0) SoonVentDeaths = 0;
    // these numbers are off because of initialization noise
    
    
    printf("As of %s evening:\n", T_to_dateName(T-1));  //(because incremented.)
//printf(" %'11d  + soon ~%'11d: Total avoidable deaths from severely ills denied Hosp.BEDS\n",
//            Total_No_Bed_Deaths,  SoonBedDeaths );
     printf("%'11d:  Total avoidable deaths from severely ills denied Hosp.BEDS\n",
            Total_No_Bed_Deaths );
//    printf("%'11d  + soon ~%'11d:  Total avoidable deaths from critically ills denied H.VENTS \n",
//            Total_No_Vent_Deaths,  SoonVentDeaths );
    printf("%'11d:  Total avoidable deaths from critically ills denied H.VENTS \n",
            Total_No_Vent_Deaths );
            
    printf("%'11d:  Total deaths of those given proper hospital treatment: \n",
            Total_InHosp_Deaths);
    printf("---------------\n");
   printf("%'11d:  GRAND TOTAL DEATHS\n", Rdead_total_count);
    
}


int main(int argc, char** argv)
{
    printf("Powered by SCRIER.org.  If you enjoy saving lives, please tip gofundme.com/f/scrier");
    double StartingBeta = DEFAULT_BETA;  // seems to be 0.42.
    double LockdownBeta = 0.0;
    ENABLE_COMMAS();
    percent_Mild     /= 100.0;  // Turn raw percent into decimal proportion.
    percent_Severe   /= 100.0;  //  "  "
    percent_Critical /= 100.0;  //  "  "
    percent_ICU_die  /= 100.0;
    
    int May1_T  = 49;
    int Jun1_T  = 81;    // if you want to do a run for all of 2020's days.
    int Jul1_T  = 111;
    int Aug1_T  = 142;
    int Oct1_T  = 203;
    int Dec31_T = 294;    // if you want to do a run for all of 2020's days.
    
    
    // Betas are basically the percent added on to the next day, on top of 100%.
    // So 0.42 means the next day will have a total of 142% of the prev day.
    // No spread at all is a Beta = 0.0.
    // Doubling each day is a Beta = 1.0, or 200% the next day.
    // Real-life in America (March '20) is currently real close to 1.42.
    // It's more complex than this, the Beta gets munged internally
    // by the number of infectious, # uninfecteds, size of population;
    // but this is close enough to be able to understand it 
    // at the beginning of a pandemic spread.  It slows down slightly later on.
    
    // Starting Beta is what we're currently at.
    // LockdownBeta is how much spread happens after ppl soft stay at home.
    // Lockdown_T is which date past March 13th ppl decide to start isolating.
    // Which day this starts affects outcomes by millions.
    
    // PARAMS:  "Banner name for that run",
    // Starting Beta spreadfactor, 
    int NoLockdown = -1;
    LockdownBeta = 0;
    do_one_run( (char*)"USA Baseline Scenario:  No Lockdowns/No Self-Isolation, B- 0.42", 
            StartingBeta, NoLockdown, LockdownBeta, Jun1_T);
    
    int Lockdown_T = 12;    //  Mar 25th. 
    LockdownBeta = 0.0;  // perfect no spread
    do_one_run( (char*)"USA Idyllic Best-Case Scenario:  0.00 at March 25", 
            StartingBeta, Lockdown_T, LockdownBeta, May1_T); 
    
    LockdownBeta = 0.2;
    do_one_run( (char*)"USA Self-Isolation Scenario:  0.20 at March 25", 
            StartingBeta, Lockdown_T, LockdownBeta, Jul1_T);    
 
    
    LockdownBeta = 0.1;
    do_one_run( (char*)"USA Inadequate 1:10 spread Lockdown Scenario:  0.10 at March 25", 
            StartingBeta, Lockdown_T, LockdownBeta, Oct1_T);     
 
    LockdownBeta = 0.07142;
    do_one_run( (char*)"USA Adequate 1:14 spread Lockdown Scenario:  0.07142 at March 25", 
            StartingBeta, Lockdown_T, LockdownBeta, Dec31_T);      
    
    //do_one_run( (char*)"Explosion", 
    //        1.0, NoLockdown, LockdownBeta, 10);
    
        /** /
    // Days from Mar 13 = 0. Subtract from Mar 13 to get this.
    int Lockdown_Mar_20 = 7;   
    LockdownBeta = 1.0/12.0;
  do_one_run( "Fast Isolation: only 1 out of 12 people spread starting Mar 20:", 
            StartingBeta, Lockdown_Mar_20, LockdownBeta, 50);
  /**/
       
    

    return 0;
}

