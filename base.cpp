#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

struct DestinationInfo {
    string destination;
    string places[5];
    string hotels[3];
    double transportCost;
    double visaFee;
};
static int totalBookings = 0;

int checkDate(string date) 
{
    if(date.length() != 10) 
        return 0;
    if(date[2] != '/' || date[5] != '/') 
        return 0;
    int day = (date[0]-'0')*10 + (date[1]-'0');
    int month = (date[3]-'0')*10 + (date[4]-'0');
    if(month < 1 || month > 12) 
        return 0;
    
    return 1;
}

vector<DestinationInfo> loadDestinations(string filename) 
{
    vector<DestinationInfo> destinations;
    ifstream file(filename.c_str());
    if(!file.is_open()) {
        cout << "ERROR: Cant open " << filename << endl;
        return destinations;
    }
    cout << "Loaded destinations from " << filename << endl;
    string line;
    getline(file, line);
    while(getline(file, line)) 
    {
        stringstream ss(line);
        DestinationInfo info;
        string temp;
        getline(ss, info.destination, ',');
        for(int i = 0; i < 5; i++) 
        {
            getline(ss, info.places[i], ',');
        }
        for(int i = 0; i < 3; i++) 
        {
            getline(ss, info.hotels[i], ',');
        }
        getline(ss, temp, ',');
        info.transportCost = atof(temp.c_str());
        getline(ss, temp);
        info.visaFee = atof(temp.c_str());
        destinations.push_back(info);
    }
    file.close();
    return destinations;
}

void showDestinations(vector<DestinationInfo> places) 
{
    cout << "\nAvailable Destinations:\n";
    for(int i = 0; i < places.size(); i++) 
    {
        cout<<(i+1)<< ". " << places[i].destination<< endl;
    }
}

// BASE CLASS
class Travel 
{
    protected:
    int tripId, num_people;    
    string source_city, dest_city, start_date, end_date;
    
    public:
    Travel(int id, string src, string dest, string sdate, string edate, int people) 
    {
        tripId = id;
        source_city = src;
        dest_city = dest;
        start_date = sdate;
        end_date = edate;
        num_people = people;
    }

    virtual double getTotalCost() = 0;
    virtual void showSummary() = 0;
    virtual void saveBooking(ofstream& file) = 0;
    
    int getNumNights() 
    {
        int sd = (start_date[0]-'0')*10 + (start_date[1]-'0');
        int sm = (start_date[3]-'0')*10 + (start_date[4]-'0');
        int sy = (start_date[6]-'0')*1000+(start_date[7]-'0')*100+(start_date[8]-'0')*10+(start_date[9]-'0');
        
        int ed = (end_date[0]-'0')*10 + (end_date[1]-'0');
        int em = (end_date[3]-'0')*10 + (end_date[4]-'0');
        int ey = (end_date[6]-'0')*1000+(end_date[7]-'0')*100+(end_date[8]-'0')*10+(end_date[9]-'0');
        if(sy == ey && sm == em && sd == ed) 
        return 0;
        int nights = 0;
        for(int y = sy; y < ey; y++) 
        {
            int days = ((y%4==0 && y%100!=0) || y%400==0) ? 366 : 365;
            nights += days;
        }
        int month_days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
        for(int m = sm; m<em; m++) 
        {
            int dim = month_days[m];
            if(m == 2 && ((ey%4==0 && ey%100!=0) || ey%400==0)) 
                dim = 29;
            nights += dim;
        }
        nights += ed - 1;
        nights -= (sd - 1);
        return nights > 0 ? nights : 0;
    }
    
    int getTripId() 
    { 
        return tripId; 
    }
    string getSource() 
    { 
        return source_city; 
    }
    string getDest() 
    { 
        return dest_city; 
    }
    
    virtual ~Travel() {}
};

//DERIVED CLASS 1
class DomesticTravel : public Travel 
{
    private:
    double hotel_cost_per_night;  
    double transport_cost;        
    
    public:
    DomesticTravel(int id, string src, string dest, string sdate, string edate, 
                   int people, double hotelcost, double tcost) : Travel(id, src, dest, sdate, edate, people) 
{
        hotel_cost_per_night = hotelcost;
        transport_cost = tcost;
    }
    
    double getTotalCost() {
        return hotel_cost_per_night * getNumNights() + transport_cost * num_people;
    }
    
    void showSummary() {
        cout <<"\n--- DOMESTIC TRIP #" << tripId << " ---" << endl;
        cout <<"From: " << source_city << " to " << dest_city << endl;
        cout <<"Nights: " <<getNumNights() << endl;
        cout <<"People: " <<num_people << endl;
        cout <<"Total: Rs." << fixed << setprecision(2) << getTotalCost() << endl;
    }
    
    void saveBooking(ofstream& file) 
    {
        file << tripId << ",Domestic," << source_city << "," << dest_city << ","<< start_date << "," << end_date << "," << num_people << ","<< getTotalCost() << endl;
    }
};
//DERIVED CLASS 2
class InternationalTravel : public Travel 
{
private:
    double hotel_cost;
    double flight_cost;
    double visa_cost_per_person;
    
public:
    InternationalTravel(int id, string src, string dest, string sdate, string edate, int people, double hotelc, double flightc, double visac): Travel(id, src, dest, sdate, edate, people) 
    {
        hotel_cost = hotelc;
        flight_cost = flightc;
        visa_cost_per_person = visac;
    }
    
    double getTotalCost() 
    {
        return hotel_cost * getNumNights() + flight_cost * num_people + 
               visa_cost_per_person * num_people;
    }
    
    void showSummary() 
    {
        cout <<"\n--- INTERNATIONAL TRIP #" << tripId <<" ---" << endl;
        cout <<"From: " << source_city << " to " << dest_city << endl;
        cout <<"Nights: " << getNumNights() << endl;
        cout <<"People: " << num_people << endl;
        cout <<"Total: Rs." <<fixed << setprecision(2) << getTotalCost() << endl;
    }
    
    void saveBooking(ofstream& file) 
    {
        file << tripId << ",International," << source_city << "," << dest_city << ","<< start_date << "," << end_date << "," << num_people << ","<< getTotalCost() << endl;
    }
};

void showhotelDetails(DestinationInfo selected) 
{
    cout << "\nTop 5 places to visit in " << selected.destination << ":\n";
    for(int i = 0; i < 5; i++) 
    {
        if(selected.places[i] != "")
            cout << (i+1) << ". " << selected.places[i] << endl;
    }
    cout << "\nTop 3 hotels in " << selected.destination << ":\n";
    for(int i = 0; i < 3; i++) 
    {
        if(selected.hotels[i] != "")
            cout << (i+1) << ". " << selected.hotels[i] << endl;
    }
}
//SAVING AND DISPLAYING BOOKINGS USING CSV FILE
void makeNewBooking(vector<Travel*>& bookings) 
{
    cout << "\n--- New Booking ---" << endl;
    int type;
    cout << "1. Domestic  2. International: ";
    cin >>type;
    cin.ignore();
    
    string source, sdate, edate;
    int people;
    double hotel_price;
    cout << "From city: ";
    getline(cin, source);
    cout << "Start date (DD/MM/YYYY): ";
    getline(cin, sdate);
    if(checkDate(sdate) == 0) 
    {
        cout <<"Invalid start date!\n";
        return;
    }
    cout <<"End date (DD/MM/YYYY): ";
    getline(cin, edate);
    if(checkDate(edate) == 0) 
    {
        cout <<"Invalid end date!\n";
        return;
    }
    cout << "Number of people: ";
    cin >> people;
    cout << "Hotel cost per night: ";
    cin >> hotel_price;
    cin.ignore();
    vector<DestinationInfo> places;
    if(type == 1) 
        places = loadDestinations("C:\\Users\\hp\\Desktop\\OOPSCaseStudy\\domestic.csv");
    else 
        places = loadDestinations("C:\\Users\\hp\\Desktop\\OOPSCaseStudy\\international.csv");    
    if(places.size() == 0) 
    {
        cout << "No destinations available!\n";
        return;
    }
    showDestinations(places);
    int choice;
    cout << "Select destination: ";
    cin >> choice;
    choice--;    
    if(choice < 0 || choice >= places.size()) 
    {
        cout <<"invalid choice!\n";
        return;
    }
    
    DestinationInfo selected = places[choice];
    showhotelDetails(selected);
    
    Travel* newTrip = NULL;
    totalBookings++;    
    if(type == 1) 
    {
        newTrip = new DomesticTravel(totalBookings, source, selected.destination, sdate, edate, people, hotel_price, selected.transportCost);
    }
    else 
    {  
        newTrip = new InternationalTravel(totalBookings, source, selected.destination,sdate, edate, people, hotel_price, selected.transportCost, selected.visaFee);
    }
    
    bookings.push_back(newTrip);
    cout << "\nBooking #"<< totalBookings <<" created!\n";
    newTrip->showSummary();
}

void showAllBookings(vector<Travel*> bookings) 
{
    cout << "\nTotal Bookings Made: " << totalBookings << endl;
    if(bookings.size() == 0) 
    {
        cout << "No bookings yet\n";
        return;
    }
    
    for(int i = 0; i < bookings.size(); i++) 
    {
        cout << "\nBooking " << (i+1) << ":\n";
        bookings[i]->showSummary();
        cout << "-------------------\n";
    }
}

void saveAll(vector<Travel*> bookings) 
{
    ofstream file("bookings.txt");
    if(!file.is_open()) 
    {
        cout << "Cant save file!\n";
        return;
    }
    
    file << "TripID,Type,From,To,Start,End,People,Cost\n";
    for(int i = 0; i < bookings.size(); i++) 
    {
        bookings[i]->saveBooking(file);
    }
    file.close();
    cout << "Saved " << bookings.size() << " bookings\n";
}

int main() 
{
    vector<Travel*> all_bookings;
    cout << "---------Travel Booking System---------\n"<<"Total bookings count (static): " << totalBookings<< " total bookings\n\n";
    int option;
    while(1) 
    {
        cout << "\n1. New Booking\n2. Show All\n3. Save\n4. Exit\n";
        cout << "Choose an option: ";
        cin >> option;
        cin.ignore();
        
        if(option == 1) 
            makeNewBooking(all_bookings);
        else if(option == 2)
            showAllBookings(all_bookings);
        else if(option == 3)
            saveAll(all_bookings);
        else if(option == 4)
            break;
        else
            cout<< "Wrong option!\n";
    }
    for(int i = 0; i < all_bookings.size(); i++) 
    {
        delete all_bookings[i];
    }
    cout << "Memory cleaned. Total bookings made: " << totalBookings << endl;
    return 0;
}