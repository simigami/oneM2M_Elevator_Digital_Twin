import Make_Dirs, Raspi_Shoot

shoot_time = 10000

if __name__ == '__main__':
    Make_Dirs.make_dirs_for_program()
    Raspi_Shoot.run_Linux(shoot_time)
