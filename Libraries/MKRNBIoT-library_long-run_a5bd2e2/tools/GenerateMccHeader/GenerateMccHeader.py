#!/usr/bin/env python3
"""
RetrievePLMNs - Retrieve Mobile Country Codes (MCC) and Mobile Network Codes (MNC) database.
RetrievePLMNs generates C++ header file or CSV file from https://en.wikipedia.org/wiki/Mobile_country_code.

Usage:
    RetrievePLMNs.py [-w FILE] [-c]

Options:
    -w FILE, --write FILE               Save data to FILE
    -c, --csv                           Output CSV

"""
from docopt import docopt

from requests import get
from bs4 import BeautifulSoup

import MobileCountryCodes as MoCoCo
import MobileIoT as IotCoLa

mmc_url = 'https://en.wikipedia.org/wiki/Mobile_country_code'
iot_url = 'https://www.gsma.com/iot/mobile-iot-commercial-launches'


def truth_table(entry, comm_filters, comm_filters_alt):
    from functools import reduce

    brand = entry['Brand']
    operator = entry['Operator']
    nation = entry['Nation']

    ts = list()

    def list_t(l):
        return reduce((lambda a, b: a or b), l)

    ts.append((brand, nation) in comm_filters)
    ts.append((brand, nation) in comm_filters_alt)
    ts.append((operator, nation) in comm_filters)
    ts.append((operator, nation) in comm_filters_alt)
    ts.append(list_t([op in brand and nat == nation for op, nat in comm_filters]))
    ts.append(list_t([op in brand and nat == nation for op, nat in comm_filters_alt]))
    ts.append(list_t([op in operator and nat == nation for op, nat in comm_filters]))
    ts.append(list_t([op in operator and nat == nation for op, nat in comm_filters_alt]))
    ts.append(list_t([brand in op and nat == nation for op, nat in comm_filters]))
    ts.append(list_t([brand in op and nat == nation for op, nat in comm_filters_alt]))
    ts.append(list_t([operator in op and nat == nation for op, nat in comm_filters]))
    ts.append(list_t([operator in op and nat == nation for op, nat in comm_filters_alt]))

    return list_t(ts)


def main(args):
    mmc_page = get(mmc_url)
    mmc_html = BeautifulSoup(mmc_page.content, 'html5lib')

    iot_page = get(iot_url)
    iot_html = BeautifulSoup(iot_page.content, 'html5lib')

    mmc_table = MoCoCo.generate_table(mmc_html)
    iot_table = IotCoLa.generate_table(iot_html)

    comm_filters = [(t['Operator'], t['Nation']) for t in iot_table]
    comm_filters_alt = [(t['Alternate'], t['Nation']) for t in iot_table if t['Alternate']]

    sorted_table = sorted(mmc_table, key=lambda l: l['Nation'])

    filtered_table = [o for o in sorted_table if truth_table(o, comm_filters, comm_filters_alt)]

    if args['--csv']:
        contents = MoCoCo.generate_csv(filtered_table)
    else:
        contents = MoCoCo.generate_cpp_header(filtered_table)

    filename = args['--write']
    if filename is not None:
        with open(f'{filename}', 'w') as output:
            output.write(contents)
    else:
        print(contents)


if __name__ == "__main__":
    arguments = docopt(__doc__, version='v0.1')
    main(arguments)
