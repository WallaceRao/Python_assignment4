import sys
import os
import pickle
import pandas as pd
from sklearn import preprocessing


vehicle_Type = ''
km = 0
year = 1990
displacement = 1000
fuel_type = 'petrol'
transmission_type = ''

print "Parameters are ", sys.argv

if(len(sys.argv) < 7):
	print "Parameters are ", sys.argv
else:
	vehicle_Type = sys.argv[1]
	km = int(sys.argv[2])
	year = int(sys.argv[3])
	displacement = int(sys.argv[4])
	fuel_type = sys.argv[5]
	transmission_type = sys.argv[6]

if(displacement < 1000):
	displacement = 1000 # We assume the displacement should not be smaller than 1000cc

if(year < 1990):
	year = 1990 # We assume the vehicle is registered after 2000

if(year > 2017):
	year = 2017 # We assume the vehicle is registered before 2017

if(km > 250000):
	km = 250000 # We assume the mileage should be smaller than 150000km

car_type_Convertible = 0
car_type_Coupe = 0
car_type_Hatchback = 0
car_type_Other = 0
car_type_RV_SUV = 0
car_type_Sedan = 0
car_type_Station_Wagon = 0
car_type_Ute = 0
car_type_Van = 0

transmission_manual = 0
transmission_automatic = 0
transmission_tiptronic = 0

if vehicle_Type == 'Hatchback':
	car_type_Hatchback = 1
if vehicle_Type == 'Van':
	car_type_Van = 1
if vehicle_Type == 'Sedan':
	car_type_Sedan = 1
if vehicle_Type == 'Ute':
	car_type_Ute = 1
if vehicle_Type == 'Station Wagon':
	car_type_Station_Wagon = 1
if vehicle_Type == 'RV/SUV':
	car_type_RV_SUV = 1
if vehicle_Type == 'Other':
	car_type_Other = 1
if vehicle_Type == 'Convertible':
	car_type_Convertible = 1
if vehicle_Type == 'Coupe':
	car_type_Coupe = 1

if transmission_type == 'automatic':
	transmission_automatic = 1
if transmission_type == 'tiptronic':
	transmission_tiptronic = 1
if transmission_type == 'manual':
	transmission_manual = 1

print "Loading Predictive Model....."
dir = os.path.dirname(os.path.abspath(__file__))
file_path = os.path.join(dir, "petrol_mod.sav")
petrol_mod = pickle.load(open(file_path, 'rb'))
file_path = os.path.join(dir, "diesel_mod.sav")
diesel_mod = pickle.load(open(file_path, 'rb'))
file_path = os.path.join(dir, "hybrid_mod.sav")
hybrid_mod = pickle.load(open(file_path, 'rb'))
file_path = os.path.join(dir, "electric_mod.sav")
electric_mod = pickle.load(open(file_path, 'rb'))


print "Predicting....."
pattern = pd.DataFrame({'car_type_Hatchback':[car_type_Hatchback], 'car_type_Van':[car_type_Van],'car_type_Sedan':[car_type_Sedan],
						'car_type_Ute':[car_type_Ute], 'car_type_Station_Wagon':[car_type_Station_Wagon],'car_type_RV_SUV':[car_type_RV_SUV],
						'car_type_Other':[car_type_Other], 'car_type_Convertible':[car_type_Convertible],'car_type_Coupe':[car_type_Coupe],
						'transmission_automatic':[transmission_automatic], 'transmission_tiptronic':[transmission_tiptronic],'transmission_manual':[transmission_manual],
						'displacement':[displacement], 'year':[year],'km':[km]})
result = ''
if(fuel_type == 'petrol'):
	prediction = petrol_mod.predict(pattern)
	result = prediction.values[0]
if(fuel_type == 'diesel'):
	prediction = diesel_mod.predict(pattern)
	result = prediction.values[0]
if(fuel_type == 'hybrid'):
	prediction = hybrid_mod.predict(pattern)
	result = prediction.values[0]
if(fuel_type == 'electric'):
	prediction = electric_mod.predict(pattern)
	result = prediction.values[0]

# Even oldest car should have a minmum price
if result < 1250 :
	result = 1250

print "result is ", result



print "Output Predictive Result..."
dir = os.path.dirname(os.path.abspath(__file__))
file_path = os.path.join(dir, "result.txt")
f = open(file_path, 'w+')
f.write(str(result))
f.close()

print "Done"
