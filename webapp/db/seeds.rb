# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rails db:seed command (or created alongside the database with db:setup).
#
# Examples:
#
#   movies = Movie.create([{ name: 'Star Wars' }, { name: 'Lord of the Rings' }])
#   Character.create(name: 'Luke', movie: movies.first)

if FamilyMember.count == 0
  FamilyMember.create(name: "Mom & Dad", blink_id: 1)
  FamilyMember.create(name: "Elias",     blink_id: 2)
  FamilyMember.create(name: "Amelia",    blink_id: 3)
  FamilyMember.create(name: "Noah",      blink_id: 4)
end
