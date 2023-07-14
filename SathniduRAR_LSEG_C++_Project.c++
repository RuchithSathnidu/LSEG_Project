#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include<string>
#include <chrono>
#include <ctime>
#include <iomanip>
using namespace std;

string getCurrentLocalTime() //get the local date and time
{
    auto now = chrono::system_clock::now();
    auto time_t_now = chrono::system_clock::to_time_t(now);
    auto tm_local = localtime(&time_t_now);

    ostringstream oss;
    oss << put_time(tm_local, "%Y-%m-%d / %H:%M:%S.");
    oss << setfill('0') << setw(3) << chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    return oss.str();
}

int main() {
  vector<string> order_list[86121]; //store the orders
  vector<string> service_list[86121]; //store the orders to provide services
  int s_num[250];//store the sells_order indexes
  int b_num[250];//store the buy_order indexes
  int s_count=0;//store the sells_order count
  int b_count=0;//store the buy_order count
  int s_l_z=0;//store the orders to provide services count
  string Transaction_Time; //store the local date and time

  ifstream input_file("ex7_input_TimePriority_SortOrder_10.csv");
  ofstream output_file("execution_rep.csv");

  if (!input_file || !output_file) //check the files are avalable
  {
    cerr << "Error opening files" << endl;
    return 1;
  }

  int count=0;//store the order count

  string line;
  while (getline(input_file, line)) //store the orders in order_list
  {
    stringstream ss(line);
    string field;
    while (getline(ss, field, ',')) {
      order_list[count].push_back(field);
    }

    count++;
  }

  string execution_line = "Order_ID,Client_Order_ID,Instrument,Side,Status,Quantity,Price,Reason,Transaction_Time"; 
  output_file << execution_line << endl;

   int i=1;
    while(i<count) 
    {
      if(order_list[i][0]=="") //check for an invalid client order id
      { 
        cout<<"Reject"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Reject," + order_list[i][3] + "," + order_list[i][4] + ",Invalid client order id," + Transaction_Time;
        output_file << execution_line << endl;
        i++;
        continue;
      }
      if(order_list[i][1]=="") //check for an invalid Instrument
      {
        cout<<"Reject"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Reject," + order_list[i][3] + "," + order_list[i][4] + ",Invalid Instrument," + Transaction_Time;
        output_file << execution_line << endl;
        i++;
        continue;
      }
      if(order_list[i][2]=="" || (stoi(order_list[i][2])!=1 && stoi(order_list[i][2])!=2)) //It contains an invalid side
      {
        cout<<"Reject"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Reject," + order_list[i][3] + "," + order_list[i][4] + ",Invalid Side," + Transaction_Time;
        output_file << execution_line << endl;
        i++;
        continue;
      }
      if(order_list[i][4]=="" || stod(order_list[i][4])<=0.00) //Its price is not greater than 0
      {
        cout<<"Reject"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Reject," + order_list[i][3] + "," + order_list[i][4] + ",Invalid Price," + Transaction_Time;
        output_file << execution_line << endl;
        i++;
        continue;
      }
      if(order_list[i][3]=="" || (stoi(order_list[i][3])%10)!=0 || stoi(order_list[i][3])<10 || stoi(order_list[i][3])>1000) //Its quantity is not a multiple of 10 and Its quantity is outside the range (min = 10 max = 1000)
      {
        cout<<"Reject"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Reject," + order_list[i][3] + "," + order_list[i][4] + ",Invalid Size," + Transaction_Time;
        output_file << execution_line << endl;
        i++;
        continue;
      }
      int r=0,new_o=1;
      while(r<s_l_z) //check if it has to be stored as a new order
      {
          if(service_list[r][2]==order_list[i][1] && service_list[r][3]!=order_list[i][2] && stoi(service_list[r][5])!=0 && (stod(service_list[r][6])==stod(order_list[i][4]) || stod(order_list[i][4])<10.00))
          {
             new_o=0;
             break;
          }
          r++;
      }
      if(new_o==1) //store the order as new order
      {
        cout<<"new"<<endl;
        Transaction_Time = getCurrentLocalTime();
        execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",New," + order_list[i][3] + "," + order_list[i][4] + ",-," + Transaction_Time;
        stringstream ss_2(execution_line);
        string temp;
        while (getline(ss_2,temp , ',')) {
             service_list[s_l_z].push_back(temp);
          }  
        output_file << execution_line << endl;
        s_l_z++;
        if(stoi(order_list[i][2])==1) //if that order is buy order
          {
            b_num[b_count]=i-1;
            b_count++;
          }
        else //if that order is sells order
          {
            s_num[s_count]=i-1;
            s_count++;  
          } 
      }
      else //if could provide the service without store as a new order
      {
        for(int z=0;z<b_count-1;z++) //sort the buy orders in descending order
        {
          for(int p=0;p<b_count-z-1;p++)
          {
            if(stod(service_list[b_num[p]][6])<stod(service_list[b_num[p+1]][6]))
            {
              vector<string> temp;
              for(int k=0;k<=8;k++)
              {
                temp.push_back(service_list[b_num[p]][k]);
              }
              for(int k=0;k<=8;k++)
              {
                service_list[b_num[p]].pop_back();
              }
              for(int k=0;k<=8;k++)
              {
                service_list[b_num[p]].push_back(service_list[b_num[p+1]][k]);
              }
              for(int k=0;k<=8;k++)
              {
                service_list[b_num[p+1]].pop_back();
              }
               for(int k=0;k<=8;k++)
              {
                service_list[b_num[p+1]].push_back(temp[k]);
              }
            }
          }
        }
         for(int z=0;z<s_count-1;z++) //sort the sales orders in ascending order
        {
          for(int p=0;p<s_count-z-1;p++)
          {
            if(stod(service_list[s_num[p]][6])>stod(service_list[s_num[p+1]][6]))
            {
              vector<string> temp;
              for(int k=0;k<=8;k++)
              {
                temp.push_back(service_list[s_num[p]][k]);
              }
              for(int k=0;k<=8;k++)
              {
                service_list[s_num[p]].pop_back();
              }
              for(int k=0;k<=8;k++)
              {
                service_list[s_num[p]].push_back(service_list[s_num[p+1]][k]);
              }
              for(int k=0;k<=8;k++)
              {
                service_list[s_num[p+1]].pop_back();
              }
               for(int k=0;k<=8;k++)
              {
                service_list[s_num[p+1]].push_back(temp[k]);
              }
              
            }
          }
        }

        int j=0;
         while(j<s_l_z) //provide the services for orders
         {
            if(service_list[j][3]!=order_list[i][2] && stod(service_list[j][6])==stod(order_list[i][4]) && service_list[j][2]==order_list[i][1]) //find the matching order to provide the service
            {
              if(stoi(service_list[j][5])==stoi(order_list[i][3])) //if quantity is same
              {
                Transaction_Time = getCurrentLocalTime();
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Fill," + order_list[i][3] + "," + order_list[i][4] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",Fill," + service_list[j][5] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]="0";
                service_list[j][6]="0.00";
                order_list[i][3]="0";
                break;
              }
              else if(stoi(service_list[j][5])>stoi(order_list[i][3])) //if quantity of new order is less
              {
                Transaction_Time = getCurrentLocalTime();
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Fill," + order_list[i][3] + "," + order_list[i][4] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",PFill," + order_list[i][3] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]=to_string(stoi(service_list[j][5])-stoi(order_list[i][3]));
                order_list[i][3]="0";
                break;
              }
              else //if quantity of new order is greater
              {
                Transaction_Time = getCurrentLocalTime();
                order_list[i][3]=to_string(stoi(order_list[i][3])-stoi(service_list[j][5]));
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",PFill," + service_list[j][5] + "," + order_list[i][4] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",Fill," + service_list[j][5] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]="0";
                service_list[j][6]="0.00";
                j++;
              }
            }

          else if(service_list[j][3]!=order_list[i][2] && stod(order_list[i][4])<10.00 && service_list[j][2]==order_list[i][1] && stoi(service_list[j][5])!=0) //find the matching order to provide the service
            {
              if(stoi(service_list[j][5])==stoi(order_list[i][3])) //if quantity is same 
              {
                Transaction_Time = getCurrentLocalTime();
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Fill," + order_list[i][3] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",Fill," + service_list[j][5] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]="0";
                service_list[j][6]="0.00";
                order_list[i][3]="0";
                cout<<order_list[i][3]<<endl;
                break;
              }
              else if(stoi(service_list[j][5])>stoi(order_list[i][3])) //if quantity of new order is less
              {
                Transaction_Time = getCurrentLocalTime();
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",Fill," + order_list[i][3] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",PFill," + order_list[i][3] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]=to_string(stoi(service_list[j][5])-stoi(order_list[i][3]));
                order_list[i][3]="0";
                cout<<order_list[i][3]<<endl;
                break;
              }
              else //if quantity of new order is greater
              {
                Transaction_Time = getCurrentLocalTime();
                order_list[i][3]=to_string(stoi(order_list[i][3])-stoi(service_list[j][5]));
                cout<<order_list[i][3]<<endl;
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",PFill," + service_list[j][5] + "," + service_list[j][6] + ",-," + Transaction_Time; 
                output_file << execution_line << endl;
                execution_line = service_list[j][0] + "," + service_list[j][1] + "," + service_list[j][2] + "," + service_list[j][3] + ",Fill," + service_list[j][5] + "," + service_list[j][6] + ",-," + Transaction_Time;
                output_file << execution_line << endl;
                service_list[j][5]="0";
                service_list[j][6]="0.00";
                j++;
              }
            } 
           else //there is no any matching orders with new order
           {
               cout<<"else"<<endl;
               j++;
           }
          cout<<j<<"   "<<i<<"    "<<count<<endl;

          }

          cout<<"end while"<<endl;

              if(stoi(order_list[i][3])!=0) //if the order is not complete
              {
                cout<<"run"<<endl;
                Transaction_Time = getCurrentLocalTime();
                execution_line = "ord" + to_string(i) + "," + order_list[i][0] + "," + order_list[i][1] + "," + order_list[i][2] + ",PFill," + order_list[i][3] + "," + order_list[i][4] + ",-," + Transaction_Time;
                        stringstream ss_2(execution_line);
                        string temp;
                        while (getline(ss_2,temp , ',')) {
                            service_list[s_l_z].push_back(temp);
                          }
                          s_l_z++;
              }
     cout<<"run"<<endl;
      }

      
      
    i++;
    }

    

  input_file.close();
  output_file.close();

  return 0;
}
