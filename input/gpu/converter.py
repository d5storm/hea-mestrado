import random
import sys


class File(object):

    def __init__(self):
        self.size = 0.0
        self.static = True
        self.static_list = []
        self.id = ""


class Job(object):

    def __init__(self):
        self.id = ""
        self.name = ""
        self.cpu_time = 0.0
        self.gpu_time = 0.0
        self.gpu = False
        self.input = []
        self.output = []


def main():
    print("123")
    file_path = sys.argv[1]
    file_end = file_path.split(".")[1]
    file_start = file_path.split(".")[0]
    gpu_path = file_start + "_gpu" + "." + file_end

    file_reader = open(file_path, "r")
    # file_writer = open(gpu_path, "w")

    lines = file_reader.readlines()

    job_dict = {}
    file_dict = {}

    static_files = 0
    dinamic_files = 0
    total_jobs = 0

    change_made = True
    while change_made:
        change_made = False
        for line in range(len(lines)):
            if lines[line] == "\n":
                del lines[line]
                change_made = True
                break
    add_line = 0
    for line in range(len(lines)):
        line_added = line + add_line
        if line_added >= len(lines):
            break
        read_line = lines[line_added]

        split_line = read_line.split(" ")
        if line_added == 0:
            static_files = int(split_line[0])
            dinamic_files = int(split_line[1])
            total_jobs = int(split_line[2])
        elif line_added < static_files + 1:
            id = split_line[0]
            size = float(split_line[1])
            static_machines_n = int(split_line[2])
            static_id = []
            for i in range(3, 3 + static_machines_n):
                static_id.append(int(split_line[i]))
            new_file = File()
            new_file.id = id
            new_file.size = size
            new_file.static = True
            new_file.static_list = static_id
            file_dict[id] = new_file
        elif line_added < dinamic_files + static_files + 1:
            id = split_line[0]
            size = float(split_line[1])
            new_file = File()
            new_file.id = id
            new_file.size = size
            new_file.static = False
            file_dict[id] = new_file
        elif line_added < dinamic_files + static_files + total_jobs:
            id = split_line[0]
            name = split_line[1]
            cpu_time = float(split_line[2])
            total_input = int(split_line[3])
            total_output = int(split_line[4])
            gpu = False
            gpu_possible = random.randint(1, 2)
            gpu_time = 0.0
            input_list = []
            output_list = []
            if gpu_possible == 1:
                gpu = True
                gpu_time = random.uniform(float(cpu_time) * 0.3, float(cpu_time) * 0.7)

            for i in range(line + 1, line + total_input + 1):
                input_list.append(lines[i])
                add_line += 1
            for i in range(line + total_input + 1, line + total_input + 1 + total_output):
                output_list.append(lines[i])
                add_line += 1

            new_job = Job()
            new_job.cpu_time = cpu_time
            new_job.gpu = gpu
            new_job.gpu_time = gpu_time
            new_job.name = name
            new_job.id = id
            new_job.output = output_list
            new_job.input = input_list

            job_dict[id] = new_job

    print("123")

main()
