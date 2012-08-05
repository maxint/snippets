import e32calendar, appuifw

# Open database
db = e32calendar.open()

# Create an appointment entry
appointment = db.add_appointment()

# Add the regular information
appointment.content = appuifw.query(u"Enter subject", "text")
appointment.location = appuifw.query(u"Enter location", "text")

# Ask the user for the start and end time
t1 = appuifw.query(u"Enter start hour", "time")
d1 = appuifw.query(u"Enter start date", "date")
t2 = appuifw.query(u"Enter end hour", "time")
d2 = appuifw.query(u"Enter end date", "date")
 
start_time = t1 + d1
end_time = t2 + d2
 
# Set the start and end time
appointment.set_time(start_time, end_time)
 
# Save the entry
appointment.commit()
