main:
	shared mutex := mutex(1)
	shared rest_count := read_integer()
	shared rest_capacity[rest_count] := 0[rest_count]
	shared rest_diners[rest_count] := 0[rest_count]
	shared rest_semaphore[rest_count]
	for local index := 0 to rest_count do
		rest_capacity[index] := read_integer()
		rest_semaphore[index] := semaphore(rest_capacity[index])

	local id := 0
	while true do
		case read_char() of
			'P': create_thread(patient(id++))
			'I': create_thread(impatient(id++))
			EOF: return

patient(id):
	local favourite_rest := id % rest_count
	lock(mutex)
	++rest_diners[favourite_rest]
	unlock(mutex)
	wait(rest_semaphore[favourite_rest])
	eat()
	lock(mutex)
	--rest_diners[favourite_rest]
	unlock(mutex)
	signal(rest_semaphore[favourite_rest])

impatient(id):
	local rest := 0
	local seeking := true
	local best_rest := id % rest_count
	local best_rest_diners := rest_diners[best_rest]
	local index := 0
	while seeking == true && index < rest_count do
		rest := (id+index) % rest_count
		if rest_diners[rest] < rest_capacity[rest]*2
			seeking := false
			best_rest := rest
		else
			if rest_diners[rest] < best_rest_diners
				best_rest_diners := rest_diners[rest]
				best_rest := rest
			walk()
		++index
	lock(mutex)
	++rest_diners[best_rest]
	unlock(mutex)
	wait(rest_semaphore[best_rest])
	eat()
	lock(mutex)
	--rest_diners[best_rest]
	unlock(mutex)
	signal(rest_semaphore[best_rest])
