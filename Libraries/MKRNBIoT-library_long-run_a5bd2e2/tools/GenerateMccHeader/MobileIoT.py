
def generate_table(html):
    table = html.find('table')
    rows = table.find_all('tr')[1:]
    deployments = []
    for row in rows:
        tds = row.find_all('td')
        if len(tds):
            _operator, nation, technology = [td.string for td in tds]
            names = _operator.split(' (')
            operator = names[0].title()
            alternate = names[1].strip(')').title() if len(names) > 1 else None

            nation = 'South Africa' if nation == 'South Afica' else nation
            nation = 'United States of America' if nation == 'North America' else nation
            nation = 'United Arab Emirates' if nation == 'UAE' else nation

            alternate = 'Slovak Telekom' if alternate == 'Slovak Telecom' else alternate
            operator = 'Vodacom' if nation == 'South Africa' and operator == 'Vodafone' else operator
            operator = 'Kt' if nation == 'South Korea' and operator == 'Korea Telecom' else operator

            deployments.append({
                'Operator': operator,
                'Alternate': alternate,
                'Nation': nation,
                'Technology': technology.title()
            })
    return deployments
