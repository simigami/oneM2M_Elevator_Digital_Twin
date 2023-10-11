import simpy

class Customer(object):
    def __init__(self, env, number):
        self.env = env
        self.number = number

        self.action = env.process(self.customer_generate())

    def customer_generate(self):
        for i in range(self.number):
            name = 'Customer-{}'.format(str(i))
            arrive = self.env.now

            print("{} arrive at {}".format(name, arrive))

            self.env.process(self.order_coffee(name, staff))

            interval_time = 10
            yield self.env.timeout(interval_time)

    def order_coffee(self, name, staff):
        with staff.request() as req:
            yield req

            ordering_duration = 30
            yield self.env.timeout(ordering_duration)
            print("{} ordered at {}".format(name, self.env.now))

        yield self.env.process(self.wait_for_coffee(name))

    def wait_for_coffee(self, name):
        waiting_duration = 30
        yield (self.env.timeout(waiting_duration))

        print("{} wait at {}".format(name, self.env.now))

if __name__ == '__main__':
    env = simpy.Environment()
    staff = simpy.Resource(env, capacity=2)
    customer = Customer(env, 10)

    env.run()